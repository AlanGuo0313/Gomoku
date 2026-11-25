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

        struct Move {
            int row;
            int col;
            char player;
        };

        vector<Move> moveHistory;
    
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
                moveHistory.push_back({row, col, currentPlayer});
                return true;
            }
            return false;
        }

        bool checkWin(int row, int col){
            char Player = board[row][col];

            // 定義四個檢查方向：橫向、直向及兩條斜線
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
        // 檢查棋盤是否已滿
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

        void resetGame() {
            board.assign(BOARD_SIZE, vector<char>(BOARD_SIZE, EMPTY));
            currentPlayer = BLACK;
            moveHistory.clear();
        }

        bool undoMove() {
            if(moveHistory.empty()) return false;

            Move lastMove = moveHistory.back();
            moveHistory.pop_back();

            board[lastMove.row][lastMove.col] = EMPTY;
            currentPlayer = lastMove.player; 
            return true;
        }
};

int main(){
    Gomoku game;
    while(true){
        game.printBoard();
        cout << "玩家 " << game.getCurrentPlayer()
             << "，請輸入列與欄（0～14，以空格分隔）"
             << "（-1 -1：悔棋，-2 -2：重新開始，-3 -3：離開）：";
        int row, col;

        if(!(cin >> row >> col)){
            if (cin.eof()) break;
            cout << "輸入錯誤，請輸入兩個整數。\n";
            cin.clear(); // 清除輸入串流的錯誤狀態
            cin.ignore(numeric_limits<streamsize>::max(), '\n'); // 丟棄本行剩餘的輸入
            continue;
        }

        if (row == -3 && col == -3) break;

        if (row == -2 && col == -2) {
            game.resetGame();
            cout << "遊戲已重新開始。\n";
            continue;
        }

        if (row == -1 && col == -1) {
            if (!game.undoMove()) {
                cout << "目前沒有可以悔棋的紀錄。\n";
            }
            continue;
        }

        if(!game.makeMove(row, col)){
            cout << "無法落子，請選擇棋盤範圍內的空位。\n";
            continue;
        }

        bool won = game.checkWin(row, col);
        if(won || game.boardFull()){
            game.printBoard();
            if (won) {
                cout << "玩家 " << game.getCurrentPlayer() << " 獲勝！\n";
            } else {
                cout << "平手！\n";
            }

            char answer;
            cout << "是否再玩一局？（y：是，n：否）：";
            if (!(cin >> answer) || (answer != 'y' && answer != 'Y')) break;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            game.resetGame();
            continue;
        }

        game.switchPlayer();
    }
}
