#pragma once

#include <algorithm>
#include <array>
#include <iostream>
#include <numeric>
#include <vector>

#include "accelerations.hpp"

namespace Connect4 {
using Bitrow = uint_fast8_t;
using Bitboard = uint_fast64_t;

constexpr auto GAME_SOLVABLE = false;
constexpr auto NUM_ROWS = 6;
constexpr auto NUM_COLS = 7;
constexpr auto GAME_EXP_FACTOR = 7.05;  // 1.41 * 5;
constexpr Bitrow BB_ALL = 0b1111111;
// constexpr Bitboard BB_ALL = (1 << (6 * 7)) - 1;
constexpr std::array<uint_fast8_t, NUM_COLS> weights = {1, 2, 3, 4, 3, 2, 1};

class State {
   public:
    using Move = uint_fast8_t;

   private:
    std::array<Move, NUM_ROWS * NUM_COLS> move_stack;
    uint_fast8_t move_count;
    // Bitboard node[2] = {0};

   public:
    std::array<std::array<Bitrow, NUM_COLS>, 2> node;
    State() {
        move_count = 0;
    }

    // STATE INTERACTIONS
    void reset() {
        std::fill(node[0].begin(), node[0].end(), 0);
        std::fill(node[1].begin(), node[1].end(), 0);
        std::fill(move_stack.begin(), move_stack.end(), 0);
        move_count = 0;
    }

    auto is_move_valid(Move move) {
        auto legals = legal_moves();
        return std::any_of(
            legals.begin(),
            legals.end(),
            [move](Move i) { return i == move; });
    }

    void mem_setup() {
        // movestack.reserve(7 * 6);
    }

    inline auto union_bitboard(int r) const -> Bitrow {
        // this function provides an occupancy number for a given row r, counting
        // downward, indexed from 0
        //
        // for example, in the case of the board:
        // . . . . . . .
        // . . . . . . .
        // . . . . . . .
        // . . . X . . .
        // . . . O . . .
        // . . O X . X .
        // unionBitboard(5) = 0b0011010, or 26
        return node[0][r] | node[1][r];
    }

    inline auto is_full() const -> bool {
        return move_count == 42;
    }

    void play(const Move col) {
        // we assume play is not called on a filled column
        // iterate upward and break at the first empty position
        for (int row = NUM_ROWS; row; row--) {
            if (!pos_filled(row - 1, col)) {
                // moveCount acts to determine which colour is played
                node[move_count & 1][row - 1] ^= (1 << col);
                break;
            }
        }
        // store the made move in the stack
        move_stack[move_count++] = col;
    }

    void unplay() {
        // decrement move counter and get the most recently played move
        Move col = move_stack[--move_count];
        // iterate downward and break at the first filled position
        for (uint_fast8_t row = 0; row < NUM_ROWS; row++) {
            if (pos_filled(row, col)) {
                // a bit is removed by XOR
                node[move_count & 1][row] ^= (1 << col);
                break;
            }
        }
    }

    void unplay(Move col) {
        // we assume pop is not called on an empty column
        // decrement move counter
        move_count--;
        // iterate downward and break at the first filled position
        for (uint_fast8_t row = 0; row < NUM_ROWS; row++) {
            if (pos_filled(row, col)) {
                // a bit is removed by XOR
                node[move_count & 1][row] ^= (1 << col);
                break;
            }
        }
    }

    void show() const {
        uint_fast8_t row, col;
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

    auto pos_filled(const uint_fast8_t row, const uint_fast8_t col) const -> bool {
        // tests if a given location is filled, indexed by the row and column
        // this is done by indexing the row in the 2D array, then performing shifts
        // to produce a mask with only the desired bit. This mask is then AND-ed with
        // the row values to test for occupancy, as 0 = false and any other value =
        // true.
        return node[0][row] & (1 << col) || node[1][row] & (1 << col);
    }

    auto player_at(const uint_fast8_t row, const uint_fast8_t col) const -> bool {
        // only valid to use if posFilled returns true
        // this function essentially performs the same job as posFilled
        // except it only checks against the first half of the array
        // and assumes that the space is filled
        return (node[0][row] & (1 << col));
        // true = X, false = O
    }

    auto probe_spot(int row, int col) const -> bool {
        // tests the bit of the most recently played side
        return (node[(move_count + 1) & 1][row] & (1 << col));
    }

    inline auto num_legal_moves() const -> uint_fast8_t {
        // this is a fast function to determine the number
        // of empty spaces on the top row
        return NUM_COLS - __builtin_popcount(union_bitboard(0));
    }

    auto legal_moves() const -> std::vector<Move> {
        // a vector to hold the generated moves
        std::vector<Move> moves(num_legal_moves());

        // this line creates an inverted occupancy for
        // the top row (0b0011000 -> 0b1100111)
        Bitrow bb = ~union_bitboard(0) & BB_ALL;

        uint_fast8_t counter = 0;

        // the following loop runs until all the occupied
        // spaces have had moves generated
        while (bb) {
            moves[counter++] = __builtin_ctz(bb);
            // clear the least significant bit set
            bb &= bb - 1;
        }
        // should be [0, 1, 2, 3, 4, 5, 6]
        // for move 1 on a 7-wide board
        return moves;
    }

    void random_play() {
        // consider inlining and stopping halfway through movegen
        std::vector<Move> moves = legal_moves();
        // assert(moves.size() != 0);
        play(moves[rand() % moves.size()]);
    }

    inline void pass_turn() {
        move_count++;
    }

    auto horizontal_term() const -> int_fast8_t {
        // check all the rows for horizontal 4-in-a-rows
        for (int row = 0; row < NUM_ROWS; row++) {
            for (int bitshift = 0; bitshift < NUM_COLS - 3; bitshift++) {
                if (((node[0][row] >> bitshift) & 0b1111) == 0b1111) {
                    return 1;
                }
                if (((node[1][row] >> bitshift) & 0b1111) == 0b1111) {
                    return -1;
                }
            }
        }
        return 0;  // no 4-in-a-rows found
    }

    auto vertical_term() const -> int_fast8_t {
        // check all the columns for vertical 4-in-a-rows
        for (int row = 0; row < NUM_ROWS - 3; row++) {
            for (int col = 0; col < NUM_COLS; col++) {
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
        int h = horizontal_term();
        if (h)
            return h;
        int v = vertical_term();
        if (v)
            return v;
        int u = diagup_term();
        if (u)
            return u;

        return diagdown_term();
    }

    void show_result() const {
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
        return (move_count & 1) ? -1 : 1;
    }

    auto get_move_count() {
        return move_count;
    }

    auto get_turn_index() {
        return move_count & 1;
    }

    auto game_running() {
        // the game is over if the board is filled up or someone has 4-in-a-row
        return (!is_full() && evaluate() == 0);
    }

    int game_result() {
        return evaluate();
    }

    int get_position_contents(int row, int col) {
        if (pos_filled(row, col)) {
            if (player_at(row, col)) {
                return 1;
            } else {
                return -1;
            }
        } else {
            return 0;
        }
    }

    auto vectorise_board() {
        std::vector<int> out(NUM_ROWS * NUM_COLS);
        for (int row = 0; row < NUM_ROWS; row++) {
            for (int col = 0; col < NUM_COLS; col++) {
                out[row * NUM_COLS + col] = get_position_contents(row, col);
            }
        }
        return out;
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
    return !(a.node[0] != b.node[0] || a.node[1] != b.node[1]);
}
}  // namespace Connect4