#include "../gomoku_ai.h"

#include <cassert>
#include <chrono>
#include <iostream>
#include <set>

constexpr Difficulty LEVELS[] = {Difficulty::Easy, Difficulty::Normal, Difficulty::Hard};

void verifyChoice(Gomoku& game, Difficulty level, int expectedRow, int expectedCol) {
    GomokuAI opponent(42);
    const auto before = game.history().size();
    const int player = game.currentPlayer();
    const auto move = opponent.chooseMove(game, level);
    assert(move.row == expectedRow && move.col == expectedCol);
    assert(game.history().size() == before);
    assert(game.currentPlayer() == player);
    assert(game.isValidMove(move.row, move.col));
}

void testOpeningAndLegality() {
    for (auto level : LEVELS) {
        Gomoku game;
        verifyChoice(game, level, 7, 7);
        assert(game.makeMove(7, 7));
        GomokuAI opponent(42);
        const auto start = std::chrono::steady_clock::now();
        const auto move = opponent.chooseMove(game, level);
        const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - start).count();
        assert(game.isValidMove(move.row, move.col));
        assert(game.history().size() == 1);
        std::cout << "Level " << static_cast<int>(level)
                  << ": " << elapsed << " ms\n";
    }
}

void testWinAndBlock() {
    for (auto level : LEVELS) {
        Gomoku win;
        // 白棋擋住左端，黑棋只能在 (7, 5) 取勝。
        for (int index = 1; index <= 4; ++index) {
            assert(win.makeMove(7, index));
            assert(win.makeMove(index == 1 ? 7 : 0, index == 1 ? 0 : index * 2));
        }
        verifyChoice(win, level, 7, 5);

        Gomoku block;
        assert(block.makeMove(7, 1));
        assert(block.makeMove(7, 0));
        for (int index = 2; index <= 3; ++index) {
            assert(block.makeMove(7, index));
            assert(block.makeMove(0, index * 2));
        }
        assert(block.makeMove(7, 4));
        verifyChoice(block, level, 7, 5);
    }
}

void testBrokenFour() {
    for (auto level : LEVELS) {
        Gomoku game;
        int filler = 0;
        for (int col : {1, 2, 4}) {
            assert(game.makeMove(7, col));
            assert(game.makeMove(0, filler++ * 2));
        }
        assert(game.makeMove(7, 5));
        verifyChoice(game, level, 7, 3);
    }
}

void testUndoRounds() {
    Gomoku game;
    assert(!game.undoToPlayer(Gomoku::BLACK));
    assert(game.makeMove(7, 7));
    // 電腦尚未回應時也能撤回玩家的一手。
    assert(game.undoToPlayer(Gomoku::BLACK));
    assert(game.history().empty());
    assert(game.makeMove(7, 7));
    assert(game.makeMove(7, 8));
    assert(game.undoToPlayer(Gomoku::BLACK));
    assert(game.history().empty());
    assert(game.currentPlayer() == Gomoku::BLACK);

    // 電腦獲勝後撤回白棋與上一手黑棋，恢復玩家回合。
    for (int step = 0; step < 5; ++step) {
        assert(game.makeMove(0, step * 2));
        assert(game.makeMove(14, step));
    }
    assert(game.winner() == Gomoku::WHITE);
    assert(game.undoToPlayer(Gomoku::BLACK));
    assert(!game.isFinished());
    assert(game.history().size() == 8);
    assert(game.currentPlayer() == Gomoku::BLACK);

    for (auto level : LEVELS) {
        Gomoku finished;
        for (int step = 0; step < 5; ++step) {
            assert(finished.makeMove(7, step));
            if (step < 4) assert(finished.makeMove(0, step * 2));
        }
        GomokuAI opponent(42);
        auto move = opponent.chooseMove(finished, level);
        assert(move.row == -1 && move.col == -1);
    }
}

void testEasyVariety() {
    Gomoku game;
    assert(game.makeMove(7, 7));
    std::set<std::pair<int, int>> choices;
    for (unsigned seed = 0; seed < 16; ++seed) {
        GomokuAI opponent(seed);
        const auto move = opponent.chooseMove(game, Difficulty::Easy);
        choices.insert({move.row, move.col});
    }
    assert(choices.size() > 1);
}

void testMidgameAndLastEmptyCell() {
    Gomoku game;
    GomokuAI opponent(17);
    // 實際走出中盤，再檢查困難搜索的結果仍合法且不改動棋局。
    for (int turn = 0; turn < 24 && !game.isFinished(); ++turn) {
        const auto move = opponent.chooseMove(game, Difficulty::Normal);
        assert(game.makeMove(move.row, move.col));
    }
    if (!game.isFinished()) {
        const auto historySize = game.history().size();
        const auto start = std::chrono::steady_clock::now();
        const auto move = opponent.chooseMove(game, Difficulty::Hard);
        assert(game.isValidMove(move.row, move.col));
        assert(game.history().size() == historySize);
        std::cout << "Hard midgame: "
                  << std::chrono::duration_cast<std::chrono::milliseconds>(
                         std::chrono::steady_clock::now() - start).count()
                  << " ms\n";
    }

    std::vector<std::pair<int, int>> blackCells;
    std::vector<std::pair<int, int>> whiteCells;
    for (int row = 0; row < Gomoku::BOARD_SIZE; ++row) {
        for (int col = 0; col < Gomoku::BOARD_SIZE; ++col) {
            ((row + 2 * col) % 4 < 2 ? blackCells : whiteCells).push_back({row, col});
        }
    }
    game.resetGame();
    for (std::size_t index = 0; index < whiteCells.size(); ++index) {
        assert(game.makeMove(blackCells[index].first, blackCells[index].second));
        assert(game.makeMove(whiteCells[index].first, whiteCells[index].second));
    }
    for (auto level : LEVELS) {
        verifyChoice(game, level, blackCells.back().first, blackCells.back().second);
    }
    assert(game.makeMove(blackCells.back().first, blackCells.back().second));
    for (auto level : LEVELS) {
        const auto move = opponent.chooseMove(game, level);
        assert(move.row == -1 && move.col == -1);
    }
}

int main() {
    testOpeningAndLegality();
    testWinAndBlock();
    testBrokenFour();
    testUndoRounds();
    testEasyVariety();
    testMidgameAndLastEmptyCell();
    std::cout << "All computer opponent tests passed.\n";
}
