#include<iostream>
#include<vector>
#include<limits>


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

        void printBoard(){
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

        bool isValidMove(int row, int col){
            if(row < 0 || row >= BOARD_SIZE || col < 0 || col >= BOARD_SIZE){
                return false;
            }
            return board[row][col] == EMPTY;
        }

        bool makeMove(int row, int col){
            if(isValidMove(row, col)){
                board[row][col] = currentPlayer;
                return true;
            }
            return false;
        }

        bool checkWin(int row, int col){
            char Player = board[row][col];

            //定義四個維度
            vector<int> dr({0,1,1,1});
            vector<int> dc({1,0,1,-1});

            for(int i = 0; i < 4; ++i){
                int count = 1;

                //正向
                for(int step = 1; step < 5; ++step){
                    int r = row + dr[i] * step;
                    int c = col + dc[i] * step;
                    if(r < 0 || r >= BOARD_SIZE || c < 0 || c >= BOARD_SIZE || board[r][c] != Player) break;
                    count++;
                }
                //反向
                for(int step = 1; step < 5; ++step){
                    int r = row - dr[i] * step;
                    int c = col - dc[i] * step;
                    if(r < 0 || r >= BOARD_SIZE || c < 0 || c >= BOARD_SIZE || board[r][c] != Player) break;
                    count++;
                }
            if(count >= 5) return true;
            }
            return false;
        }
        //檢查是否平手
        bool boardFull(){
            for(int i = 0; i < BOARD_SIZE; ++i){
                for(int j = 0; j < BOARD_SIZE; ++j){
                    if(board[i][j] == EMPTY){
                        return false;
                    }
                }
            }
            return true;
        }

        char getCurrentPlayer() const {return currentPlayer;}
        void switchPlayer() {
            if(currentPlayer == BLACK) currentPlayer = WHITE;
            else currentPlayer = BLACK;
        }
};

int main(){
    Gomoku game;
    while(true){
        game.printBoard();
        cout << "Player " << game.getCurrentPlayer() << ", enter your move (row col): ";
        int row, col;

        if(!(cin >> row >> col)){
            cout << "Invalid input. Please enter two integers.\n";
            cin.clear(); // clear the error flag
            cin.ignore(numeric_limits<streamsize>::max(), '\n'); // discard invalid input
            continue;
        }

        if(!game.makeMove(row, col)){
            cout << "Move could not be made. Try again.\n";
            continue;
        }

        if(game.checkWin(row, col)){
            game.printBoard();
            cout << "Player " << game.getCurrentPlayer() << " wins!\n";
            break;
        }

        if(game.boardFull()){
            game.printBoard();
            cout << "It's a draw!\n";
            break;
        }

        game.switchPlayer();
    }
}