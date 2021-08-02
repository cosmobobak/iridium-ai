#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <iostream>
#include <numeric>
#include <vector>

namespace Connect4 {
using Bitrow = uint_fast8_t;
using Bitboard = unsigned long long;
constexpr std::array<int, 7> weights = {1, 2, 3, 4, 3, 2, 1};

template < int NUM_ROWS = 6, int NUM_COLS = 7 >
class State {
   public:
    using Move = uint_fast8_t;
    static constexpr auto GAME_SOLVABLE = NUM_COLS * NUM_ROWS <= 25;
    static constexpr auto GAME_EXP_FACTOR = 10;  // 1.41 * 5;
    static constexpr int BB_ALL = (1 << NUM_COLS) - 1;
    static constexpr auto MAX_GAME_LENGTH = NUM_ROWS * NUM_COLS;
    static constexpr auto NUM_ACTIONS = NUM_COLS;

    static_assert(!(NUM_ROWS == 6 && NUM_COLS == 7) || BB_ALL == 0b1111111, "Invalid BB_ALL");

   private:
    std::array<std::array<Bitrow, NUM_COLS>, 2> node = {0};
    // std::array<Move, MAX_GAME_LENGTH> move_stack = {0};
    int_fast8_t move_count;

   public:
    State() {
        move_count = 0;
    }

    // GETTERS
    [[nodiscard]] auto get_turn() const -> int {
        return (move_count & 1) ? -1 : 1;
    }

    [[nodiscard]] auto get_move_count() const -> int {
        return move_count;
    }

    [[nodiscard]] auto get_turn_index() const -> int {
        return move_count & 1;
    }

    [[nodiscard]] auto get_node() const -> const std::array<std::array<Bitrow, NUM_COLS>, 2>& {
        return node;
    }

    // SETTERS
    void set_move_count(int n) {
        move_count = n;
    }

    // PREDICATES
    [[nodiscard]] auto is_full() const -> bool {
        return move_count == MAX_GAME_LENGTH;
    }

    [[nodiscard]] auto is_game_over() const -> bool {
        return is_full() || evaluate();
    }

    [[nodiscard]] auto is_legal(Move move) const -> bool {
        auto legals = legal_moves();
        return std::any_of(
            legals.begin(),
            legals.end(),
            [move](Move i) { return i == move; });
    }

    // MOVE GENERATION
    [[nodiscard]] auto num_legal_moves() const -> size_t {
        // this is a fast function to determine the number
        // of empty spaces on the top row
        return NUM_COLS - __builtin_popcount(union_bitboard(0));
    }

    [[nodiscard]] auto legal_moves() const -> std::vector<Move> {
        int bb = node[0][0] | node[1][0];
        // a vector to hold the generated moves
        std::vector<Move> moves(NUM_COLS - __builtin_popcount(bb));

        // this line creates an inverted occupancy for
        // the top row (0b0011000 -> 0b1100111)
        bb = ~bb & BB_ALL;

        int counter = 0;

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
        // the union of the top rows
        int bb = node[0][0] | node[1][0];
        int num_moves = NUM_COLS - __builtin_popcount(bb);

        // the chosen move
        // assert(num_moves != 0);
        int choice = rand() % num_moves;

        // this line creates an inverted occupancy for
        // the top row (0b0011000 -> 0b1100111)
        bb = ~bb & BB_ALL;

        // the loop runs until
        // we hit the chosen move
        while (choice) {
            // clear the least significant bit set
            bb &= bb - 1;
            --choice;
        }

        if (!bb) {
            std::cout << "you fucked up: " << std::endl;
            auto original_bb = node[0][0] | node[1][0];
            std::cout << "node: " << original_bb << std::endl;
            std::cout << "num_moves: " << NUM_COLS - __builtin_popcount(original_bb) << std::endl;
        }
        auto move = __builtin_ctz(bb);

        play(move);
    }

    // DATA VIEWS
    [[nodiscard]] auto union_bitboard(int r) const -> Bitrow {
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

    [[nodiscard]] auto pos_filled(int row, int col) const -> bool {
        // tests if a given location is filled, indexed by the row and column
        // this is done by indexing the row in the 2D array, then performing shifts
        // to produce a mask with only the desired bit. This mask is then AND-ed with
        // the row values to test for occupancy, as 0 = false and any other value =
        // true.
        return node[0][row] & (1 << col) || node[1][row] & (1 << col);
    }

    [[nodiscard]] auto player_at(int row, int col) const -> bool {
        // only valid to use if posFilled returns true
        // this function essentially performs the same job as posFilled
        // except it only checks against the first half of the array
        // and assumes that the space is filled
        return node[0][row] & (1 << col);
        // true = X, false = O
    }

    [[nodiscard]] auto probe_spot(int row, int col) const -> bool {
        // tests the bit of the most recently played side
        return node[(move_count + 1) & 1][row] & (1 << col);
    }

    // STATE INTERACTIONS
    void mem_setup() {
        // move_stack.reserve(7 * 6);
    }

    void pass_turn() {
        move_count++;
    }

    void unpass_turn() {
        move_count--;
    }

    void reset() {
        std::fill(node[0].begin(), node[0].end(), 0);
        std::fill(node[1].begin(), node[1].end(), 0);
        // std::fill(move_stack.begin(), move_stack.end(), 0);
        move_count = 0;
    }

    void play(int col) {
        // assert(!pos_filled(0, col));
        // we assume play is not called on a filled column
        // iterate upward and break at the first empty position
        int row = NUM_ROWS;
        while (pos_filled(row - 1, col)) {
            row--;
        }
        // moveCount acts to determine which colour is played
        // assert that row - 1 is in bounds
        assert(row - 1 >= 0 && row - 1 < NUM_ROWS);
        node[move_count & 1][row - 1] ^= (1 << col);
        // store the made move in the stack
        // move_stack[move_count++] = col;
        move_count++;
    }

    void unplay() {
        // // decrement move counter and get the most recently played move
        // Move col = move_stack[--move_count];
        // // assert(pos_filled(NUM_ROWS - 1, col));
        // // iterate downward and break at the first filled position
        // int row = 0;
        // while (!pos_filled(row, col)) {
        //     row++;
        // }
        // // a bit is removed by XOR
        // node[move_count & 1][row] ^= (1 << col);
    }

    void unplay(int col) {
        // assert(pos_filled(NUM_ROWS - 1, col));
        // we assume pop is not called on an empty column
        // decrement move counter
        move_count--;
        // iterate downward and break at the first filled position
        int row = 0;
        while (!pos_filled(row, col)) {
            row++;
        }
        // a bit is removed by XOR
        node[move_count & 1][row] ^= (1 << col);
    }

    // EVALUATION
   private:
    [[nodiscard]] auto horizontal_term() const -> int {
        // check all the rows for horizontal 4-in-a-rows
        for (int row = 0; row < NUM_ROWS; row++) {
            for (int bitshift = 0; bitshift < NUM_COLS - 3; bitshift++) {
                if (((node[(move_count + 1) & 1][row] >> bitshift) & 0b1111) == 0b1111) {
                    return -get_turn();
                }
            }
        }
        return 0;  // no 4-in-a-rows found
    }

    [[nodiscard]] auto vertical_term() const -> int {
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

    [[nodiscard]] auto diagup_term() const -> int {
        // check all the upward diagonals for 4-in-a-rows
        for (int row = 3; row < NUM_ROWS; row++) {
            for (int col = 0; col < NUM_COLS - 3; col++) {
                if (probe_spot(row, col) && probe_spot(row - 1, col + 1) && probe_spot(row - 2, col + 2) && probe_spot(row - 3, col + 3)) {
                    // if we have four adjacent filled positions
                    return -get_turn();
                }
            }
        }
        return 0;  // no 4-in-a-rows found
    }

    [[nodiscard]] auto diagdown_term() const -> int {
        // check all the downward diagonals for 4-in-a-rows
        for (int row = 0; row < NUM_ROWS - 3; row++) {
            for (int col = 0; col < NUM_COLS - 3; col++) {
                if (probe_spot(row, col) && probe_spot(row + 1, col + 1) && probe_spot(row + 2, col + 2) && probe_spot(row + 3, col + 3)) {
                    // if we have four adjacent filled positions
                    return -get_turn();
                }
            }
        }
        return 0;  // no 4-in-a-rows found
    }
   public:
    [[nodiscard]] auto evaluate() const -> int {
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

    [[nodiscard]] auto in_bounds(int row, int col) const -> bool {
        return row >= 0 && row < NUM_ROWS && col >= 0 && col < NUM_COLS;
    }

    [[nodiscard]] auto heuristic_value() const -> int {
        int val = 0;
        for (int row = 0; row < NUM_ROWS; row++) {
            for (int i = 0; i < NUM_COLS; i++) {
                val += pos_filled(row, i) * (player_at(row, i) ? 1 : -1) * weights[i];
            }
        }
        return val;  // use some sort of central weighting approach
    }

    // DATA GENERATION
    [[nodiscard]] auto game_running() const -> bool {
        // the game is over if the board is filled up or someone has 4-in-a-row
        return (!is_full() && evaluate() == 0);
    }

    [[nodiscard]] auto game_result() const -> int {
        if (!is_game_over()) {
            std::cerr << "game_result() called on in-progress game.\n";
            return -2;
        }
        return evaluate();
    }

    [[nodiscard]] auto get_position_contents(int row, int col) const -> int {
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

    [[nodiscard]] auto vectorise_board() const -> std::vector<int> {
        std::vector<int> out(NUM_ROWS * NUM_COLS);
        for (int row = 0; row < NUM_ROWS; row++) {
            for (int col = 0; col < NUM_COLS; col++) {
                out[row * NUM_COLS + col] = get_position_contents(row, col);
            }
        }
        return out;
    }

    // I/O
    void show() const {
        int row, col;
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

    void show_legal_moves() const {
        std::vector<Move> legals = legal_moves();
        std::vector<Move> shiftedLegals;
        std::transform(legals.begin(), legals.end(), std::back_inserter(shiftedLegals), [](Move n) { return n + 1; });
        std::cout << "Your legal moves are: ";
        std::cout << "{ ";
        for (auto i : shiftedLegals) {
            std::cout << (int)i;
            std::cout << ", ";
        }
        std::cout << "}";
    }

    [[nodiscard]] auto get_player_move() const -> Move {
        show_legal_moves();
        std::cout << "\n--> ";
        int move;
        std::cin >> move;
        while (!is_legal(move - 1)) {
            std::cout << "invalid move.\n";
            show();
            std::cin >> move;
        }
        return move - 1;
    }

    friend auto operator==(const State& a, const State& b) -> bool {
        return a.node == b.node;
    }
};
}  // namespace Connect4