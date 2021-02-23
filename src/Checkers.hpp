#include <iostream>
#include <vector>

namespace Checkers {

using Move = int;

class State {
   public:
    char node[6][7] = {
        {'.', '.', '.', '.', '.', '.', '.'},
        {'.', '.', '.', '.', '.', '.', '.'},
        {'.', '.', '.', '.', '.', '.', '.'},
        {'.', '.', '.', '.', '.', '.', '.'},
        {'.', '.', '.', '.', '.', '.', '.'},
        {'.', '.', '.', '.', '.', '.', '.'},
    };
    int turn = 1;
    int nodes = 0;
    char players[2] = {'X', 'O'};
    std::vector<Move> movestack;

    void reset() {
        for (int row = 0; row < 6; row++) {
            for (int col = 0; col < 7; col++) {
                node[row][col] = '.';
            }
        }
    }

    auto is_full() -> bool  //WORKING
    {
        for (int row = 0; row < 6; row++) {
            for (int col = 0; col < 7; col++) {
                if (node[row][col] == '.') {
                    return false;
                }
            }
        }
        return true;
    }

    void show()  //WORKING
    {
        int row, col;
        for (row = 0; row < 6; ++row) {
            for (col = 0; col < 7; ++col) {
                std::cout << node[row][col] << ' ';
            }
            std::cout << '\n';
        }
        std::cout << '\n';
    }

    auto legal_moves() -> std::vector<int> {
        std::vector<int> moves;
        int ordering[] = {3, 4, 2, 5, 1, 6, 0};
        for (int col = 0; col < 7; col++) {
            if (node[0][ordering[col]] == '.') {
                moves.push_back(ordering[col]);
            }
        }
        return moves;
    }

    void random_play() {
        int col;
        col = rand() % 7;
        while (node[0][col] != '.') {
            col = rand() % 7;
        }
        play(col);
    }

    auto pos_filled(int col) -> bool {
        return (node[0][col] != '.');
    }

    void pass_turn() {
        turn = -turn;
    }

    void play(int col)  //WORKING
    {
        for (int row = 0; row < 6; row++) {
            if (node[row][col] != '.') {
                if (turn == 1) {
                    node[row - 1][col] = players[0];
                    turn = -1;
                    break;
                } else {
                    node[row - 1][col] = players[1];
                    turn = 1;
                    break;
                }
            } else if (row == 5) {
                if (turn == 1) {
                    node[row][col] = players[0];
                    turn = -1;
                } else {
                    node[row][col] = players[1];
                    turn = 1;
                }
            }
        }
        movestack.push_back(col);
    }

    void unplay()  //WORKING
    {
        int col = movestack.back();
        movestack.pop_back();
        for (int row = 0; row < 6; row++) {
            if (node[row][col] != '.') {
                node[row][col] = '.';
                break;
            }
        }
        if (turn == 1) {
            turn = -1;
        } else {
            turn = 1;
        }
    }

    auto horizontal_term() -> int {
        for (int row = 0; row < 6; row++) {
            for (int col = 0; col < 4; col++) {
                if (node[row][col] == node[row][col + 1] &&
                    node[row][col + 1] == node[row][col + 2] &&
                    node[row][col + 2] == node[row][col + 3]) {
                    if (node[row][col] == players[0]) {
                        return 1;
                    } else if (node[row][col] == players[1]) {
                        return -1;
                    }
                }
            }
        }
        return 0;
    }

    auto vertical_term() -> int {
        for (int row = 0; row < 3; row++) {
            for (int col = 0; col < 7; col++) {
                if (node[row][col] == node[row + 1][col] &&
                    node[row + 1][col] == node[row + 2][col] &&
                    node[row + 2][col] == node[row + 3][col]) {
                    if (node[row][col] == players[0]) {
                        return 1;
                    } else if (node[row][col] == players[1]) {
                        return -1;
                    }
                }
            }
        }
        return 0;
    }

    auto diagup_term() -> int {
        for (int row = 3; row < 6; row++) {
            for (int col = 0; col < 4; col++) {
                if (node[row][col] == node[row - 1][col + 1] &&
                    node[row - 1][col + 1] == node[row - 2][col + 2] &&
                    node[row - 2][col + 2] == node[row - 3][col + 3]) {
                    if (node[row][col] == players[0]) {
                        return 1;
                    } else if (node[row][col] == players[1]) {
                        return -1;
                    }
                }
            }
        }
        return 0;
    }

    auto diagdown_term() -> int {
        for (int row = 0; row < 3; row++) {
            for (int col = 0; col < 4; col++) {
                if (node[row][col] == node[row + 1][col + 1] &&
                    node[row + 1][col + 1] == node[row + 2][col + 2] &&
                    node[row + 2][col + 2] == node[row + 3][col + 3]) {
                    if (node[row][col] == players[0]) {
                        return 1;
                    } else if (node[row][col] == players[1]) {
                        return -1;
                    }
                }
            }
        }
        return 0;
    }

    auto evaluate() -> int {
        int v, h, u, d;
        v = vertical_term();
        if (v) {
            return v;
        }
        h = horizontal_term();
        if (h) {
            return h;
        }
        u = diagup_term();
        if (u) {
            return u;
        }
        d = diagdown_term();
        if (d) {
            return d;
        }

        return 0;
    }

    void show_result()  //WORKING
    {
        int r;
        r = evaluate();
        if (r == 0) {
            std::cout << "1/2-1/2" << '\n';
        } else if (r > 0) {
            std::cout << "1-0" << '\n';
        } else {
            std::cout << "0-1" << '\n';
        }
    }

    auto is_game_over() -> int {
        if (evaluate() != 0 || is_full()) {
            return true;
        } else {
            return false;
        }
    }
};
}  // namespace Checkers