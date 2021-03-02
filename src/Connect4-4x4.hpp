#pragma once

#include <algorithm>
#include <array>
#include <iostream>
#include <numeric>
#include <vector>

#include "accelerations.hpp"

namespace Connect4x4 {

constexpr auto GAME_SOLVABLE = true;
constexpr auto NUM_ROWS = 4;
constexpr auto NUM_COLS = 4;
constexpr auto GAME_EXP_FACTOR = 1.41 * 5;
constexpr uint_fast8_t BB_ALL = 0b1111;
constexpr std::array<uint_fast8_t, 4> weights = {2, 1, 1, 2};

class State {
   public:
    using Move = uint_fast8_t;
    using Bitrow = uint_fast8_t;
    std::array<std::array<Bitrow, 4>, 2> bbnode = {0};
    std::array<Move, NUM_ROWS * NUM_COLS> movestack;
    uint_fast8_t movecount = 0;

    void mem_setup() {
        // movestack.reserve(4 * 4);
    }

    auto union_bb(uint_fast8_t r) const -> Bitrow {
        return bbnode[0][r] | bbnode[1][r];
    }

    auto is_full() const -> bool
    {
        return union_bb(0) == BB_ALL;
    }

    void play(Move col)  //WORKING
    {
        movestack[movecount] = col;
        for (uint_fast8_t row = NUM_ROWS; row; row--) {
            if (!pos_filled(row - 1, col)) {
                bbnode[movecount & 1][row - 1] ^= (1 << col);
                break;
            }
        }
        movecount++;
    }

    void unplay()  //WORKING
    {
        movecount--;
        Move col = movestack[movecount];
        for (uint_fast8_t row = 0; row < NUM_ROWS; row++) {
            if (pos_filled(row, col)) {
                bbnode[movecount & 1][row] ^= (1 << col);
                break;
            }
        }
    }

    auto get_turn() const -> int_fast8_t {
        if (movecount & 1)
            return -1;
        else
            return 1;
    }

    void show() const 
    {
        uint_fast8_t row, col;
        for (row = 0; row < 4; ++row) {
            for (col = 0; col < 4; ++col) {
                if (pos_filled(row, col)) {
                    if (player_at(row, col))
                        std::cout << "X ";
                    else
                        std::cout << "O ";
                } else
                    std::cout << ". ";
            }
            std::cout << '\n';
        }
        std::cout << '\n';
    }

    auto pos_filled(int row, int col) const -> bool {
        return bbnode[0][row] & (1 << col) || bbnode[1][row] & (1 << col);
    }

    auto player_at(int row, int col) const -> bool  //only valid to use if pos_filled() returns true, true = x, false = y
    {
        return (bbnode[0][row] & (1 << col));
    }

    auto probe_spot(int row, int col) const -> bool {
        // tests the bit of the most recently played side
        return (bbnode[(movecount + 1) & 1][row] & (1 << col));
    }

    auto num_legal_moves() const -> uint_fast8_t {
        return NUM_COLS - __builtin_popcount(bbnode[0][0] | bbnode[1][0]);
    }

    auto legal_moves() const -> std::vector<Move> {
        std::vector<Move> moves;
        moves.reserve(num_legal_moves());
        uint_fast8_t bb = ~(bbnode[0][0] | bbnode[1][0]) & BB_ALL;
        while (bb) {
            moves.push_back(__builtin_ctz(bb));
            bb &= bb - 1;  // clear the least significant bit set
        }
        return moves;
    }

    void random_play() {
        std::vector<Move> moves = legal_moves();
        play(moves[rand() % moves.size()]);
    }

    inline void pass_turn() {
        movecount++;
    }

    auto horizontal_term() const -> int_fast8_t {
        // check all the rows for horizontal 4-in-a-rows
        for (uint_fast8_t row = 0; row < NUM_ROWS; row++) {
            for (uint_fast8_t bitshift = 0; bitshift < NUM_COLS - 3; bitshift++) {
                if (((bbnode[0][row] >> bitshift) & 0b1111) == 0b1111) {
                    return 1;
                }
                if (((bbnode[1][row] >> bitshift) & 0b1111) == 0b1111) {
                    return -1;
                }
            }
        }
        return 0;  // no 4-in-a-rows found
    }

    auto vertical_term() const -> int_fast8_t {
        // check all the columns for vertical 4-in-a-rows
        for (uint_fast8_t row = 0; row < NUM_ROWS - 3; row++) {
            for (uint_fast8_t col = 0; col < NUM_COLS; col++) {
                if (probe_spot(row, col) && probe_spot(row + 1, col) && probe_spot(row + 2, col) && probe_spot(row + 3, col)) {
                    // if we have four adjacent filled positions
                    return -get_turn();
                }
            }
        }
        return 0;  // no 4-in-a-rows found
    }

    auto diagup_term() const -> int_fast8_t {
        // check all the upward diagonals for 4-in-a-rows
        for (uint_fast8_t row = 3; row < NUM_ROWS; row++) {
            for (uint_fast8_t col = 0; col < NUM_COLS - 3; col++) {
                if (probe_spot(row, col) && probe_spot(row - 1, col + 1) && probe_spot(row - 2, col + 2) && probe_spot(row - 3, col + 3)) {
                    // if we have four adjacent filled positions
                    return -get_turn();
                }
            }
        }
        return 0;  // no 4-in-a-rows found
    }

    auto diagdown_term() const -> int_fast8_t {
        // check all the downward diagonals for 4-in-a-rows
        for (uint_fast8_t row = 0; row < NUM_ROWS - 3; row++) {
            for (uint_fast8_t col = 0; col < NUM_COLS - 3; col++) {
                if (probe_spot(row, col) && probe_spot(row + 1, col + 1) && probe_spot(row + 2, col + 2) && probe_spot(row + 3, col + 3)) {
                    // if we have four adjacent filled positions
                    return -get_turn();
                }
            }
        }
        return 0;  // no 4-in-a-rows found
    }

    auto evaluate() const -> int_fast8_t {
        int_fast8_t v = vertical_term();
        if (v)
            return v;
        int_fast8_t h = horizontal_term();
        if (h)
            return h;
        int_fast8_t u = diagup_term();
        if (u)
            return u;
        int_fast8_t d = diagdown_term();
        if (d)
            return d;

        return 0;
    }

    void show_result() const  //WORKING
    {
        uint_fast8_t r;
        r = evaluate();
        if (r == 0) {
            std::cout << "1/2-1/2" << '\n';
        } else if (r > 0) {
            std::cout << "1-0" << '\n';
        } else {
            std::cout << "0-1" << '\n';
        }
    }

    auto is_game_over() const -> bool {
        return (is_full() || evaluate());
    }

    auto heuristic_value() -> uint_fast8_t {
        uint_fast8_t val = 0;
        for (uint_fast8_t row = 0; row < NUM_ROWS; row++) {
            for (uint_fast8_t i = 0; i < NUM_COLS; i++) {
                val += pos_filled(row, i) * (player_at(row, i) ? 1 : -1) * weights[i];
            }
        }
        return val;  // use some sort of central weighting approach
    }

    auto get_player_move() -> Move {
        const std::vector<Move> legals = legal_moves();
        std::vector<Move> shiftedLegals;
        std::transform(legals.begin(), legals.end(), std::back_inserter(shiftedLegals), [](Move n) { return n + 1; });
        std::cout << "Your legal moves are: " << string(shiftedLegals) << "\n--> ";
        Move pos;
        std::cin >> pos;
        while (std::none_of(legals.begin(), legals.end(), [pos](Move m) { return m == (pos - 1); })) {
            std::cout << "invalid move.\n";
            show();
            std::cin >> pos;
        }
        return pos - 1;
    }
};

bool operator==(State a, State b) {
    return !(a.bbnode[0] != b.bbnode[0] || a.bbnode[1] != b.bbnode[1]);
}
}  // namespace Connect4x4