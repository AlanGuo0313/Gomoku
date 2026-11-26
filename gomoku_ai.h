#pragma once

#include "gomoku_game.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <random>
#include <vector>

enum class Difficulty { Easy, Normal, Hard };

// 本機棋力引擎：簡單保留隨機性，普通評估攻防，困難向前搜索。
// 不修改真正的棋局，視窗可以在背景執行這個計算。
class GomokuAI {
public:
    struct Position {
        int row = -1;
        int col = -1;
    };

    explicit GomokuAI(unsigned seed = std::random_device{}()) : random_(seed) {}

    Position chooseMove(const Gomoku& game, Difficulty difficulty) {
        if (game.isFinished()) return {};
        for (int row = 0; row < SIZE; ++row) {
            for (int col = 0; col < SIZE; ++col) {
                board_[row][col] = game.stoneAt(row, col);
            }
        }
        const int player = game.currentPlayer();
        auto moves = candidates(player);
        if (moves.empty()) return {};

        // 所有難度都知道先取勝，以及阻擋對手下一手的五連。
        for (const auto& move : moves) {
            if (potential(move.row, move.col, player) >= WIN) return move.position();
        }
        for (const auto& move : moves) {
            if (potential(move.row, move.col, 3 - player) >= WIN) return move.position();
        }

        if (difficulty == Difficulty::Easy) {
            const int count = std::min<int>(6, moves.size());
            std::uniform_int_distribution<int> pick(0, count - 1);
            return moves[pick(random_)].position();
        }
        if (difficulty == Difficulty::Normal) return moves.front().position();

        // 逐層加深，僅使用完整完成的一層結果；時間與節點上限避免久候。
        deadline_ = std::chrono::steady_clock::now() + std::chrono::milliseconds(650);
        nodes_ = 0;
        interrupted_ = false;
        Position best = moves.front().position();
        const int rootCount = std::min<int>(12, moves.size());
        for (int depth = 2; depth <= 3; ++depth) {
            int bestScore = -INFINITY_SCORE;
            Position iterationBest = best;
            for (int index = 0; index < rootCount; ++index) {
                const auto& move = moves[index];
                board_[move.row][move.col] = player;
                int score = -search(3 - player, depth - 1, -INFINITY_SCORE, INFINITY_SCORE);
                board_[move.row][move.col] = Gomoku::EMPTY;
                if (interrupted_) break;
                if (score > bestScore) {
                    bestScore = score;
                    iterationBest = move.position();
                }
            }
            if (interrupted_) break;
            best = iterationBest;
        }
        return best;
    }

private:
    static constexpr int SIZE = Gomoku::BOARD_SIZE;
    static constexpr int WIN = 1000000;
    static constexpr int INFINITY_SCORE = 100000000;
    std::array<std::array<int, SIZE>, SIZE> board_{};
    std::mt19937 random_;
    std::chrono::steady_clock::time_point deadline_;
    int nodes_ = 0;
    bool interrupted_ = false;

    struct Candidate {
        int row;
        int col;
        int score;
        Position position() const { return {row, col}; }
    };

    static bool inside(int row, int col) {
        return row >= 0 && row < SIZE && col >= 0 && col < SIZE;
    }

    // 假設在空位落子，評估四方向的連線與含空缺的五格視窗。
    int potential(int row, int col, int player) const {
        constexpr int directions[4][2] = {{0, 1}, {1, 0}, {1, 1}, {1, -1}};
        int score = 0;
        for (const auto& direction : directions) {
            int length = 1;
            int openEnds = 0;
            for (int sign : {-1, 1}) {
                int r = row + direction[0] * sign;
                int c = col + direction[1] * sign;
                while (inside(r, c) && board_[r][c] == player) {
                    ++length;
                    r += direction[0] * sign;
                    c += direction[1] * sign;
                }
                if (inside(r, c) && board_[r][c] == Gomoku::EMPTY) ++openEnds;
            }
            if (length >= 5) return WIN;
            if (length == 4) score += openEnds == 2 ? 60000 : openEnds == 1 ? 12000 : 0;
            if (length == 3) score += openEnds == 2 ? 5000 : openEnds == 1 ? 400 : 0;
            if (length == 2) score += openEnds == 2 ? 250 : openEnds == 1 ? 40 : 0;

            // 能包含本次落子的五格視窗，也能辨識「XX.XX」等缺口。
            for (int start = -4; start <= 0; ++start) {
                int stones = 0;
                bool blocked = false;
                for (int offset = start; offset < start + 5; ++offset) {
                    int r = row + direction[0] * offset;
                    int c = col + direction[1] * offset;
                    if (!inside(r, c) || (board_[r][c] != Gomoku::EMPTY &&
                                         board_[r][c] != player)) {
                        blocked = true;
                        break;
                    }
                    if (offset == 0 || board_[r][c] == player) ++stones;
                }
                if (!blocked) {
                    constexpr int weights[6] = {0, 1, 12, 120, 1800, WIN};
                    score += weights[stones];
                }
            }
        }
        return score;
    }

    std::vector<Candidate> candidates(int player) const {
        std::vector<Candidate> moves;
        bool hasStone = false;
        for (int row = 0; row < SIZE; ++row) {
            for (int col = 0; col < SIZE; ++col) {
                if (board_[row][col] != Gomoku::EMPTY) {
                    hasStone = true;
                    continue;
                }
                bool nearby = false;
                for (int dr = -2; dr <= 2 && !nearby; ++dr) {
                    for (int dc = -2; dc <= 2; ++dc) {
                        if (inside(row + dr, col + dc) &&
                            board_[row + dr][col + dc] != Gomoku::EMPTY) {
                            nearby = true;
                            break;
                        }
                    }
                }
                if (!nearby) continue;
                int attack = potential(row, col, player);
                int defense = potential(row, col, 3 - player);
                int centrality = 14 - std::abs(row - 7) - std::abs(col - 7);
                moves.push_back({row, col, attack + defense * 11 / 10 + centrality});
            }
        }
        if (!hasStone) moves.push_back({7, 7, 0});
        std::stable_sort(moves.begin(), moves.end(),
                         [](const Candidate& a, const Candidate& b) { return a.score > b.score; });
        return moves;
    }

    int evaluate(int player) const {
        int ownBest = 0;
        int opponentBest = 0;
        for (int row = 0; row < SIZE; ++row) {
            for (int col = 0; col < SIZE; ++col) {
                if (board_[row][col] != Gomoku::EMPTY) continue;
                ownBest = std::max(ownBest, potential(row, col, player));
                opponentBest = std::max(opponentBest, potential(row, col, 3 - player));
            }
        }
        // 輪到我走時，自己的立即勝利優先於對方的威脅。
        if (ownBest >= WIN) return WIN;
        return ownBest - opponentBest * 11 / 10;
    }

    int search(int player, int depth, int alpha, int beta) {
        if (++nodes_ > 3500 || std::chrono::steady_clock::now() >= deadline_) {
            interrupted_ = true;
            return 0;
        }
        if (depth == 0) return evaluate(player);
        auto moves = candidates(player);
        if (moves.empty()) return 0;
        int best = -INFINITY_SCORE;
        const int count = std::min<int>(8, moves.size());
        for (int index = 0; index < count; ++index) {
            const auto& move = moves[index];
            if (potential(move.row, move.col, player) >= WIN) return WIN + depth;
            board_[move.row][move.col] = player;
            int value = -search(3 - player, depth - 1, -beta, -alpha);
            board_[move.row][move.col] = Gomoku::EMPTY;
            if (interrupted_) return 0;
            best = std::max(best, value);
            alpha = std::max(alpha, value);
            if (alpha >= beta) break;
        }
        return best;
    }
};
