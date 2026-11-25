#pragma once

#include <array>
#include <vector>

// 純遊戲邏輯：不依賴 Windows，也不負責畫面或滑鼠輸入。
class Gomoku {
public:
    static constexpr int BOARD_SIZE = 15;
    static constexpr int EMPTY = 0;
    static constexpr int BLACK = 1;
    static constexpr int WHITE = 2;

    struct Move {
        int row;
        int col;
        int player;
    };

    bool isValidMove(int row, int col) const {
        return isInsideBoard(row, col) && board_[row][col] == EMPTY;
    }

    bool makeMove(int row, int col) {
        if (finished_ || !isValidMove(row, col)) {
            return false;
        }

        board_[row][col] = currentPlayer_;
        history_.push_back({row, col, currentPlayer_});

        if (checkWin(row, col)) {
            winner_ = currentPlayer_;
            finished_ = true;
        } else if (history_.size() == BOARD_SIZE * BOARD_SIZE) {
            finished_ = true;
        } else {
            currentPlayer_ = (currentPlayer_ == BLACK) ? WHITE : BLACK;
        }
        return true;
    }

    bool undoMove() {
        if (history_.empty()) {
            return false;
        }

        const Move lastMove = history_.back();
        history_.pop_back();
        board_[lastMove.row][lastMove.col] = EMPTY;

        // 悔棋後，讓下這一手的玩家重新落子；已結束的棋局也能繼續。
        currentPlayer_ = lastMove.player;
        winner_ = EMPTY;
        finished_ = false;
        return true;
    }

    void resetGame() {
        board_ = {};
        history_.clear();
        currentPlayer_ = BLACK;
        winner_ = EMPTY;
        finished_ = false;
    }

    int stoneAt(int row, int col) const {
        return isInsideBoard(row, col) ? board_[row][col] : EMPTY;
    }

    int currentPlayer() const { return currentPlayer_; }
    int winner() const { return winner_; }
    bool isFinished() const { return finished_; }
    const std::vector<Move>& history() const { return history_; }

private:
    std::array<std::array<int, BOARD_SIZE>, BOARD_SIZE> board_{};
    std::vector<Move> history_;
    int currentPlayer_ = BLACK;
    int winner_ = EMPTY;
    bool finished_ = false;

    static bool isInsideBoard(int row, int col) {
        return row >= 0 && row < BOARD_SIZE && col >= 0 && col < BOARD_SIZE;
    }

    bool checkWin(int row, int col) const {
        // 每個方向都同時向前、向後計數，包含剛下的棋子。
        constexpr int directions[4][2] = {
            {0, 1},   // 橫向
            {1, 0},   // 直向
            {1, 1},   // 左上至右下
            {1, -1}   // 右上至左下
        };

        for (const auto& direction : directions) {
            int count = 1;

            for (int sign : {-1, 1}) {
                int nextRow = row + direction[0] * sign;
                int nextCol = col + direction[1] * sign;

                while (isInsideBoard(nextRow, nextCol) &&
                       board_[nextRow][nextCol] == board_[row][col]) {
                    ++count;
                    nextRow += direction[0] * sign;
                    nextCol += direction[1] * sign;
                }
            }

            if (count >= 5) {
                return true;
            }
        }
        return false;
    }
};
