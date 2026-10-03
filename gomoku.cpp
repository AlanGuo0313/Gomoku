#include<iostream>
#include<vector>
#include<string>

using namespace std;

const int BOARD_SIZE = 15;
const char EMPTY = '.';
const char BLACK = 'X';
const char WHITE = 'O';

class Gomoku {
    private:
        vector<vector<char>> board;
        char currentPlayer;
    
    public:
        Gomoku(){
            board = vector<vector<char>>(BOARD_SIZE, vector<char>(BOARD_SIZE, EMPTY));
            currentPlayer = BLACK;
        }

        void printBorad(){
            cout << "\n   ";
            for(int i = 0; i < BOARD_SIZE; ++i){
                cout << (i < 10?" ": "") << i << " ";
            }
            cout << "\n";

            for(int i = 0; i < BOARD_SIZE; ++i){
                cout << (i < 10?" " : "") << i << " "; 
                for(int j = 0; j < BOARD_SIZE; ++j){
                    cout << " " << board[i][j] << " ";
                }
                cout << "\n";
            }
        }
};

int main(){
    Gomoku game;
    game.printBorad();
}