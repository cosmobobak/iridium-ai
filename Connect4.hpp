#pragma once

#include <algorithm>
#include <array>
#include <iostream>
#include <numeric>
#include <vector>

#include "accelerations.hpp"

namespace Connect4 {

constexpr auto GAME_SOLVABLE = false;
constexpr auto NUM_ROWS = 6;
constexpr auto NUM_COLS = 7;
constexpr auto gameexpfactor = 5;
constexpr int_fast8_t BB_ALL = 0b1111111;
constexpr std::array<int_fast8_t, NUM_COLS> weights = {1, 2, 3, 4, 3, 2, 1};
using Move = int_fast8_t;
using Bitrow = int_fast8_t;
using Bitboard = uint_fast64_t;

class State {
   public:
    std::array<std::array<Bitrow, NUM_ROWS>, 2> bbnode = {0};
    Bitboard node[2] = {0};
    int_fast16_t movecount = 0;
    int_fast32_t nodes = 0;
    std::array<Move, NUM_ROWS * NUM_COLS> movestack;

    void mem_setup() {
        // movestack.reserve(7 * 6);
    }

    auto union_bb(const int_fast8_t r) const -> Bitrow {
        return bbnode[0][r] | bbnode[1][r];
    }

    auto XXunion_bb() const -> Bitboard {
        return node[0] | node[1];
    }

    auto is_full() const -> bool {
        return union_bb(0) == BB_ALL;
    }

    auto XXis_full() const -> bool {
        return (XXunion_bb() & BB_ALL) == BB_ALL;
    }

    void show() const {
        int_fast8_t row, col;
        for (row = 0; row < NUM_ROWS; ++row) {
            for (col = 0; col < NUM_COLS; ++col) {
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

    auto pos_filled(const int_fast8_t row, const int_fast8_t col) const -> bool {
        return bbnode[0][row] & (1 << col) || bbnode[1][row] & (1 << col);
    }

    auto pos_filled(const int_fast8_t col) const -> bool {
        return bbnode[0][0] & (1 << col) || bbnode[1][0] & (1 << col);
    }

    auto XXpos_filled(const int_fast8_t row, const int_fast8_t col) const -> bool {
        const int_fast16_t i = (row + 1) * 7 + col;
        return node[0] & (1 << i) || node[1] & (1 << i);
    }

    auto XXpos_filled(const int_fast16_t i) const -> bool {
        return node[0] & (1 << i) || node[1] & (1 << i);
    }

    auto player_at(const int_fast8_t row, const int_fast8_t col) const -> bool  //only valid to use if pos_filled() returns true, true = x, false = y
    {
        return (bbnode[0][row] & (1 << col));
    }

    // auto XXplayer_at(const int_fast8_t row, const int_fast8_t col) const -> bool {
    //     //only valid to use if pos_filled() returns true, true = x, false = y
    //     return node[0] & (1 << ((row + 1) * 7 + col));
    // }

    // auto XXplayer_at(const int_fast16_t i) const -> bool {
    //     //only valid to use if pos_filled() returns true, true = x, false = y
    //     return node[0] & (1 << i);
    // }

    auto num_legal_moves() const -> int_fast8_t {
        return NUM_COLS - popcount(bbnode[0][0] | bbnode[1][0]);
    }

    // auto XXnum_legal_moves() const -> int_fast8_t {
    //     return 7 - popcount(union_bb() & BB_ALL);
    // }

    auto legal_moves() const -> std::vector<Move> {
        std::vector<Move> moves;
        moves.reserve(NUM_COLS);
        int_fast8_t bb = ~(bbnode[0][0] | bbnode[1][0]) & BB_ALL;
        while (bb) {
            moves.push_back(lsb(bb));
            bb &= bb - 1;  // clear the least significant bit set
        }
        return moves;
    }

    // auto XXlegal_moves() const -> std::vector<Move> {
    //     std::vector<Move> moves;
    //     moves.reserve(7);
    //     int_fast8_t bb = ~(node[0] | node[1]) & BB_ALL;
    //     while (bb) {
    //         moves.push_back(lsb(bb));
    //         bb &= bb - 1;  // clear the least significant bit set
    //     }
    //     return moves;
    // }

    void random_play() {
        std::vector<Move> moves = legal_moves();
        play(moves[rand() % moves.size()]);
    }

    inline void pass_turn() {
        movecount++;
    }

    void play(const Move col)  //WORKING
    {
        movestack[movecount] = col;
        for (int_fast8_t row = NUM_ROWS; row; row--) {
            if (!pos_filled(row - 1, col)) {
                bbnode[movecount & 1][row - 1] ^= (1 << col);
                break;
            }
        }
        movecount++;
    }

    // void XXplay(const Move col)  //WORKING
    // {
    //     long move = 1L << height[col]++;  // (1)
    //     bitboard[counter & 1] ^= move;    // (2)
    //     moves[counter++] = col;           // (3)
    //     turn = -turn;
    //     movestack.push_back(col);
    // }

    void unplay()  //WORKING
    {
        movecount--;
        Move col = movestack[movecount];
        for (int_fast8_t row = 0; row < NUM_ROWS; row++) {
            if (pos_filled(row, col)) {
                bbnode[movecount & 1][row] ^= (1 << col);
                break;
            }
        }
    }

    auto horizontal_term() const -> int_fast8_t {
        // check all the rows for horizontal 4-in-a-rows
        for (int_fast8_t row = 0; row < NUM_ROWS; row++) {
            for (int_fast8_t bitshift = 0; bitshift < 4; bitshift++) {
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
        for (int_fast8_t row = 0; row < NUM_ROWS - 3; row++) {
            for (int_fast8_t col = 0; col < NUM_COLS; col++) {
                if (pos_filled(row, col) &&
                    pos_filled(row + 1, col) &&
                    pos_filled(row + 2, col) &&
                    pos_filled(row + 3, col)) {
                    if (player_at(row, col) == player_at(row + 1, col) &&
                        player_at(row + 1, col) == player_at(row + 2, col) &&
                        player_at(row + 2, col) == player_at(row + 3, col)) {
                        if (player_at(row, col))
                            return 1;
                        else
                            return -1;
                    }
                }
            }
        }
        return 0;
    }

    auto diagup_term() const -> int_fast8_t {
        for (int_fast8_t row = 3; row < NUM_ROWS; row++) {
            for (int_fast8_t col = 0; col < NUM_COLS - 3; col++) {
                if (pos_filled(row, col) &&
                    pos_filled(row - 1, col + 1) &&
                    pos_filled(row - 2, col + 2) &&
                    pos_filled(row - 3, col + 3)) {
                    if (player_at(row, col) == player_at(row - 1, col + 1) &&
                        player_at(row - 1, col + 1) == player_at(row - 2, col + 2) &&
                        player_at(row - 2, col + 2) == player_at(row - 3, col + 3)) {
                        if (player_at(row, col))
                            return 1;
                        else
                            return -1;
                    }
                }
            }
        }
        return 0;
    }

    auto diagdown_term() const -> int_fast8_t {
        for (int_fast8_t row = 0; row < NUM_ROWS - 3; row++) {
            for (int_fast8_t col = 0; col < NUM_COLS - 3; col++) {
                if (pos_filled(row, col) &&
                    pos_filled(row + 1, col + 1) &&
                    pos_filled(row + 2, col + 2) &&
                    pos_filled(row + 3, col + 3)) {
                    if (player_at(row, col) == player_at(row + 1, col + 1) &&
                        player_at(row + 1, col + 1) == player_at(row + 2, col + 2) &&
                        player_at(row + 2, col + 2) == player_at(row + 3, col + 3)) {
                        if (player_at(row, col))
                            return 1;
                        else
                            return -1;
                    }
                }
            }
        }
        return 0;
    }

    auto evaluate() const -> int_fast8_t {
        int_fast8_t h = horizontal_term();
        if (h)
            return h;
        int_fast8_t v = vertical_term();
        if (v)
            return v;
        int_fast8_t u = diagup_term();
        if (u)
            return u;
        int_fast8_t d = diagdown_term();
        if (d)
            return d;

        return 0;
    }

    void show_result() const {
        int_fast8_t r;
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

    auto heuristic_value() -> int_fast8_t {
        int_fast8_t val = 0;
        for (int_fast8_t row = 0; row < NUM_ROWS; row++) {
            for (int_fast8_t i = 0; i < NUM_COLS; i++) {
                val += pos_filled(row, i) * (player_at(row, i) ? 1 : -1) * weights[i];
            }
        }
        return val;  // use some sort of central weighting approach
    }

    auto get_player_move() -> Move {
        const std::vector<Move> legals = legal_moves();
        std::vector<Move> shiftedLegals;
        std::transform(legals.begin(), legals.end(), std::back_inserter(shiftedLegals), [](Move n) { return n + 1; });
        std::cout << "Your legal moves are: ";
        showvec(shiftedLegals);
        std::cout << "\n--> ";
        int pos;
        std::cin >> pos;
        while (std::none_of(legals.begin(), legals.end(), [pos](Move m) { return m == (pos - 1); })) {
            std::cout << "invalid move.\n";
            show();
            std::cin >> pos;
        }
        return pos - 1;
    }

    auto get_turn() const -> int_fast8_t {
        if (movecount & 1)
            return -1;
        else
            return 1;
    }

    // struct HashFunction {
    //     auto operator()(const State& pos) const -> size_t {
    //         size_t rowHash = std::hash<decltype(pos.bbnode[0])>()(pos.bbnode[0]);
    //         size_t colHash = std::hash<decltype(pos.bbnode[1])>()(pos.bbnode[1]) << 1;
    //         return rowHash ^ colHash;
    //     }
    // };
};

bool operator==(State a, State b) {
    return !(a.bbnode[0] != b.bbnode[0] || a.bbnode[1] != b.bbnode[1]);
}
}  // namespace Connect4