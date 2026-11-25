#include "../gomoku_game.h"

#include <cassert>
#include <iostream>
#include <utility>

void testInvalidMoves() {
    Gomoku game;
    assert(!game.undoMove());
    assert(!game.makeMove(-1, 0));
    assert(!game.makeMove(0, Gomoku::BOARD_SIZE));
    assert(game.history().empty());
    assert(game.makeMove(7, 7));
    assert(game.currentPlayer() == Gomoku::WHITE);
    assert(!game.makeMove(7, 7));
    assert(game.history().size() == 1);
}

void testWinDirection(int rowStep, int colStep) {
    Gomoku game;
    for (int step = 0; step < 5; ++step) {
        assert(game.makeMove(7 + rowStep * step, 7 + colStep * step));
        if (step < 4) {
            assert(!game.isFinished());
            assert(game.makeMove(0, step * 2));
        }
    }
    assert(game.isFinished());
    assert(game.winner() == Gomoku::BLACK);
    assert(!game.makeMove(14, 14));
    assert(game.undoMove());
    assert(!game.isFinished());
    assert(game.winner() == Gomoku::EMPTY);
    assert(game.currentPlayer() == Gomoku::BLACK);
    assert(game.makeMove(7 + rowStep * 4, 7 + colStep * 4));
    assert(game.winner() == Gomoku::BLACK);
    game.resetGame();
    assert(game.history().empty());
    assert(game.currentPlayer() == Gomoku::BLACK);
    assert(!game.isFinished());
    for (int row = 0; row < Gomoku::BOARD_SIZE; ++row) {
        for (int col = 0; col < Gomoku::BOARD_SIZE; ++col) {
            assert(game.stoneAt(row, col) == Gomoku::EMPTY);
        }
    }
}

void testWhiteWin() {
    Gomoku game;
    for (int step = 0; step < 5; ++step) {
        assert(game.makeMove(0, step * 2));
        assert(game.makeMove(14, step));
    }
    assert(game.winner() == Gomoku::WHITE);
}

void testLongLine() {
    Gomoku game;
    // 最後補上中間的空缺，一次形成六連。
    for (int col : {0, 1, 2, 4, 5}) {
        assert(game.makeMove(7, col));
        assert(game.makeMove(0, col * 2));
    }
    assert(!game.isFinished());
    assert(game.makeMove(7, 3));
    assert(game.winner() == Gomoku::BLACK);
}

void testDraw() {
    Gomoku game;
    // 此滿盤排列的任一方向均不會出現五連，黑白各為 113、112 顆。
    std::vector<std::pair<int, int>> blackCells;
    std::vector<std::pair<int, int>> whiteCells;
    for (int row = 0; row < Gomoku::BOARD_SIZE; ++row) {
        for (int col = 0; col < Gomoku::BOARD_SIZE; ++col) {
            const bool black = (row + 2 * col) % 4 < 2;
            (black ? blackCells : whiteCells).push_back({row, col});
        }
    }
    assert(blackCells.size() == 113);
    assert(whiteCells.size() == 112);
    for (std::size_t index = 0; index < blackCells.size(); ++index) {
        assert(game.makeMove(blackCells[index].first, blackCells[index].second));
        if (index < whiteCells.size()) {
            assert(game.makeMove(whiteCells[index].first, whiteCells[index].second));
        }
    }
    assert(game.isFinished());
    assert(game.winner() == Gomoku::EMPTY);
    assert(game.history().size() == 225);
    assert(game.undoMove());
    assert(!game.isFinished());
}

int main() {
    testInvalidMoves();
    testWinDirection(0, 1);
    testWinDirection(1, 0);
    testWinDirection(1, 1);
    testWinDirection(1, -1);
    testWhiteWin();
    testLongLine();
    testDraw();
    std::cout << "All game logic tests passed.\n";
}
