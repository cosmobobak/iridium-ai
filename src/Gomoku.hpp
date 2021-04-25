#pragma once

#include <array>
#include <bitset>
#include <iostream>
#include <vector>

#include "accelerations.hpp"

namespace Gomoku {

class State {
   public:
    using Move = uint_fast8_t;
    static constexpr auto GAME_SOLVABLE = false;
    static constexpr auto GAME_EXP_FACTOR = 8;
    static constexpr auto WIDTH = 8;
    static constexpr auto HEIGHT = 8;
    static constexpr auto MAX_GAME_LENGTH = WIDTH * HEIGHT;
    static constexpr std::array<char, 2> players = {'X', 'O'};

   private:
    std::array<unsigned long long, 2> node = {0};
    int move_count;
    std::array<Move, MAX_GAME_LENGTH> move_stack = {0};

   public:
    State() {
        move_count = 0;
    }

    auto get_turn() const -> int {
        return (move_count & 1) ? -1 : 1;
    }

    auto get_move_count() const -> int {
        return move_count;
    }

    auto get_turn_index() const -> int {
        return move_count & 1;
    }

    auto is_full() const -> bool {
        return move_count == MAX_GAME_LENGTH;
    }
    
    void reset() {
        std::fill(node.begin(), node.end(), 0);
        std::fill(move_stack.begin(), move_stack.end(), 0);
        move_count = 0;
    }

    void show() const {
        int row, col;
        for (row = 0; row < HEIGHT; ++row) {
            for (col = 0; col < WIDTH; ++col) {
                if (pos_filled(row * WIDTH + col)) {
                    if (player_at(row * WIDTH + col))
                        std::cout << "X ";
                    else
                        std::cout << "0 ";
                } else
                    std::cout << ". ";
            }
            std::cout << '\n';
        }
        std::cout << '\n';
    }

    void mem_setup() {
        // move_stack.reserve(MAX_GAME_LENGTH);
    }

    auto get_node() const -> const std::array<unsigned long long, 2>& {
        return node;
    }

    auto player_at(int i) const -> bool {
        //only valid to use if pos_filled() returns true, true = x, false = y
        return node[0] & (1ULL << (i));
    }

    auto player_at(int row, int col) const -> bool {
        // return node[0].test(row * WIDTH + col);
        return node[0] & (1ULL << (row * WIDTH + col));
    }

    auto pos_filled(int i) const -> bool {
        return (node[0] | node[1]) & (1ULL << (i));
    }

    auto pos_filled(int row, int col) const -> bool {
        return (node[0] | node[1]) & (1ULL << (row * WIDTH + col));
    }

    auto probe_spot(int i) const -> bool {
        // tests the bit of the most recently played side
        return node[(move_count + 1) & 1] & (1ULL << i);
    }

    auto probe_spot(int row, int col) const -> bool {
        // tests the bit of the most recently played side
        return node[(move_count + 1) & 1] & (1ULL << (row * WIDTH + col));
    }

    auto num_legal_moves() const -> uint_fast16_t {
        return WIDTH * HEIGHT - __builtin_popcountll(node[0] | node[1]);
    }

    auto legal_moves() const -> std::vector<Move> {
        unsigned long long bb = node[0] | node[1];
        // a vector to hold the generated moves
        std::vector<Move> moves(WIDTH * HEIGHT - __builtin_popcountll(bb));

        // this line creates an inverted occupancy for
        // the top row (0b0011000 -> 0b1100111)
        bb = ~bb;

        int counter = 0;

        // the following loop runs until all the occupied
        // spaces have had moves generated
        while (bb) {
            moves[counter++] = __builtin_ctzll(bb);
            // clear the least significant bit set
            bb &= bb - 1;
        }
        // should be [0, 1, 2, 3, 4, 5, 6]
        // for move 1 on a 7-wide board
        return moves;
    }

    void random_play() {
        // the union of the top rows
        unsigned long long bb = node[0] | node[1];
        int num_moves = WIDTH * HEIGHT - __builtin_popcountll(bb);

        // the chosen move
        // assert(num_moves != 0);
        int choice = rand() % num_moves;

        // this line creates an inverted occupancy for
        // the top row (0b0011000 -> 0b1100111)
        bb = ~bb;

        // the loop runs until
        // we hit the chosen move
        while (choice--) {
            // clear the least significant bit set
            bb &= bb - 1;
        }

        play(__builtin_ctzll(bb));
    }

    
    void pass_turn() {
        move_count++;
    }

    void unpass_turn() {
        move_count--;
    }

    void play(int i) {
        // move_count acts to determine which colour is played
        node[move_count & 1] ^= (1ULL << i);
        // store the made move in the stack
        move_stack[move_count++] = i;
    }

    void unplay() {
        // decrement move counter and get the most recently played move
        int i = move_stack[--move_count];
        // a bit is removed by XOR
        node[move_count & 1] ^= (1ULL << i);
    }

    auto is_game_over() const -> bool {
        return is_full() || evaluate();
    }

    auto horizontal_term() const -> int {
        // iterates the starting positions of the rows
        for (Move row = 0; row < WIDTH * HEIGHT; row += WIDTH) {
            for (Move i = row; i < row + WIDTH - 4; i++) {
                if (probe_spot(i) &&
                    probe_spot(i + 1) &&
                    probe_spot(i + 2) &&
                    probe_spot(i + 3) &&
                    probe_spot(i + 4)) {
                    return -get_turn();
                }
            }
        }
        return 0;
    }

    auto vertical_term() const -> int {
        // iterates the starting positions of the columns
        for (Move col = 0; col < WIDTH; col++) {
            // this line below could be fucky
            for (Move i = col; i < col + (WIDTH * (HEIGHT - 4)); i += WIDTH) {
                if (probe_spot(i) &&
                    probe_spot(i + WIDTH * 1) &&
                    probe_spot(i + WIDTH * 2) &&
                    probe_spot(i + WIDTH * 3) &&
                    probe_spot(i + WIDTH * 4)) {
                    return -get_turn();
                }
            }
        }
        return 0;
    }

    auto diagdown_term() const -> int {
        // iterates the starting positions of the rows
        for (Move row = 0; row < HEIGHT - 4; row++) {
            for (Move col = 0; col < WIDTH - 4; col++) {
                if (probe_spot(row, col) &&
                    probe_spot(row + 1, col + 1) &&
                    probe_spot(row + 2, col + 2) &&
                    probe_spot(row + 3, col + 3) &&
                    probe_spot(row + 4, col + 4)) {
                    return -get_turn();
                }
            }
        }
        return 0;
    }

    auto diagup_term() const -> int {
        // iterates the starting positions of the rows
        for (Move row = 4; row < HEIGHT; row++) {
            for (Move col = 0; col < WIDTH - 4; col++) {
                if (probe_spot(row, col) &&
                    probe_spot(row - 1, col + 1) &&
                    probe_spot(row - 2, col + 2) &&
                    probe_spot(row - 3, col + 3) &&
                    probe_spot(row - 4, col + 4)) {
                    return -get_turn();
                }
            }
        }
        return 0;
    }

    auto evaluate() const -> int {
        int v = vertical_term();
        if (v)
            return v;
        int h = horizontal_term();
        if (h)
            return h;
        int u = diagup_term();
        if (u)
            return u;
        int d = diagdown_term();
        if (d)
            return d;

        return 0;
    }

    void show_result() {
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

    auto get_player_move() const -> Move {
        const std::vector<Move> legals = legal_moves();
        std::vector<Move> shiftedLegals;
        std::transform(legals.begin(), legals.end(), std::back_inserter(shiftedLegals), [](Move n) { return n + 1; });
        std::vector<Move> rows, cols;
        for (auto m : legals) {
            rows.push_back(m / WIDTH + 1);
            cols.push_back(m % WIDTH + 1);
        }

        std::cout << "Your legal moves are: " << zipstring(rows, cols) << "\n";
        Move row, col, pos;
        std::cout << "Enter row: ";
        std::cin >> row;
        std::cout << "Enter col: ";
        std::cin >> col;
        pos = (row - 1) * WIDTH + (col - 1);
        while (std::none_of(legals.begin(), legals.end(), [pos](Move m) { return m == (pos); })) {
            std::cout << "invalid move.\n";
            std::cout << "Enter row: ";
            std::cin >> row;
            std::cout << "Enter col: ";
            std::cin >> col;
            pos = (row - 1) * WIDTH + (col - 1);
        }
        return pos;
    }

    auto heuristic_value() const -> uint_fast8_t {
        return 0;
    }
};

bool operator==(const State a, const State b) {
    return a.get_node() == b.get_node();
}
}  // namespace Gomoku