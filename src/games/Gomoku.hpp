#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <iostream>
#include <numeric>
#include <vector>

#include "../utilities/BitMatrix.hpp"
#include "../utilities/rng.hpp"

namespace Gomoku {

template <typename T>
auto zipstring(std::vector<T> v1, std::vector<T> v2) -> std::string {
    assert(v1.size() == v2.size());
    std::string builder;
    builder.append("{ ");
    for (size_t i = 0; i < v1.size(); i++) {
        // builder.append((std::to_string)(i));
        builder += '(';
        builder.append(std::to_string(v1[i]));
        builder += ',';
        builder.append(std::to_string(v2[i]));
        builder += ')';
        builder.append(", ");
    }
    builder.append("}");
    return builder;
}

template < int WIDTH = 8, int HEIGHT = 8 >
class State {
   public:
    using Move = uint_fast8_t;
    using BB = BitMatrix<WIDTH, HEIGHT>;
    static constexpr auto GAME_SOLVABLE = false;
    static constexpr auto GAME_EXP_FACTOR = 8;
    static constexpr auto MAX_GAME_LENGTH = WIDTH * HEIGHT;
    static constexpr auto NUM_ACTIONS = WIDTH * HEIGHT;
    static constexpr std::array<char, 2> players = {'X', 'O'};

   private:
    std::array<BB, 2> node;
    int move_count;
    // std::array<Move, MAX_GAME_LENGTH> move_stack = {0};

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
        node[0].reset();
        node[1].reset();
        // std::fill(move_stack.begin(), move_stack.end(), 0);
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

    void set_move_count(int n) {
        move_count = n;
    }

    auto get_node() const -> const std::array<BB, 2>& {
        return node;
    }

    auto player_at(int i) const -> bool {
        //only valid to use if pos_filled() returns true, true = x, false = y
        return node[0].test_bit(i);
    }

    auto player_at(int row, int col) const -> bool {
        return node[0].test_bit(row * WIDTH + col);
    }

    auto pos_filled(int i) const -> bool {
        return (node[0] | node[1]).test_bit(i);
    }

    auto pos_filled(int row, int col) const -> bool {
        return (node[0] | node[1]).test_bit(row * WIDTH + col);
    }

    auto probe_spot(int i) const -> bool {
        // tests the bit of the most recently played side
        return node[(move_count + 1) & 1].test_bit(i);
    }

    auto probe_spot(int row, int col) const -> bool {
        // tests the bit of the most recently played side
        return node[(move_count + 1) & 1].test_bit(row * WIDTH + col);
    }

    auto num_legal_moves() const -> size_t {
        return WIDTH * HEIGHT - (node[0] | node[1]).popcount();
    }

    auto legal_moves() const -> std::vector<Move> {
        std::vector<Move> moves;

        auto bb = node[0] | node[1];

        // put bits in all the empty slots to play
        bb.flip();

        // this inserts all the empty slots into the moves vector
        bb.bitscan(moves);
        
        return moves;
    }

    void random_play() {
        auto bb = node[0] | node[1];
        bb.flip();
        int idx = rng::random_int(bb.popcount());

        auto move = bb.nth_bit(idx);

        play(move);
    }

    void pass_turn() {
        move_count++;
    }

    void unpass_turn() {
        move_count--;
    }

    void play(int i) {
        // move_count acts to determine which colour is played
        node[move_count & 1].set_bit(i);
        // store the made move in the stack
        // move_stack[move_count] = i;
        move_count++;
    }

    void unplay() {
        // decrement move counter and get the most recently played move
        // -------------- int i = move_stack[--move_count];
        // a bit is removed by XOR
        // -------------- node[move_count & 1] ^= (1ULL << i);
    }

    void unplay(int i) {
        // decrement move counter and get the most recently played move
        --move_count;
        // a bit is removed
        node[move_count & 1].clear_bit(i);
    }

    auto is_game_over() const -> bool {
        return is_full() || evaluate();
    }
   private:
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

    auto nevaluate() const -> int {
        auto eval = 0;
        if (node[0].has_n_in_a_row(5)){
            eval = 1;
        } else if (node[1].has_n_in_a_row(5)) {
            eval = -1;
        }
        return eval;
    }

   public:
    auto evaluate() const -> int {
        if (!move_count)
            return 0;

        auto out = 0;
        int v, h, u, d;
        v = vertical_term();
        if (v) {
            out = v;
            goto ret;
        }
        h = horizontal_term();
        if (h) {
            out = h;
            goto ret;
        }
        u = diagup_term();
        if (u) {
            out = u;
            goto ret;
        }
        d = diagdown_term();
        if (d) {
            out = d;
            goto ret;
        }

       ret:
        out = 0;

        auto new_eval = nevaluate();

        printf("new eval: %d\n", new_eval);
        printf("old eval: %d\n", out);

        return out;
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
        int row, col, pos;
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

    friend auto operator==(const State& a, const State& b) -> bool {
        return a.node == b.node;
    }
};

}  // namespace Gomoku