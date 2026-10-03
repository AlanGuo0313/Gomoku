#include<iostream>
#include<vector>
#include<string>

using namespace std;

const int BORAD_SIZE = 15;
const char EMPTY = '.';
const char BLACK = 'X';
const char WHITE = 'O';

class Gomoku {
    private:
        vector<vector<char>> borad;
        char currentPlayer;
    
    public:
        Gomoku(){
            borad = vector<vector<char>>(BORAD_SIZE, vector<char>(BORAD_SIZE, EMPTY));
            currentPlayer = BLACK;
        }

        void printBorad(){
            cout << "\n   ";
            for(int i = 0; i < BORAD_SIZE; ++i){
                cout << (i < 10?" ": "") << i << "";
            }
            cout << "\n";
        }
};

int main(){
    cout << "hello";
}