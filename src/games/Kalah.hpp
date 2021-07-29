#pragma once

#include <algorithm>
#include <array>
#include <iostream>
#include <numeric>
#include <vector>

namespace Kalah {

class State {
   public:
    // a move is an integer that represents an ordered list of pot choices.
    // e.g. the move 0b110001 (110, 001 --> 5, 1) means "move from pot 1, then pot 5".
    using Move = unsigned long long;
   private:
    static constexpr auto NUM_SLOTS = 14;
    static constexpr auto P1_STORE = 0;
    static constexpr auto P2_STORE = 7;
    std::array<int8_t, NUM_SLOTS> slots;
    int move_count;

   public:
    static constexpr auto GAME_SOLVABLE = false;
    static constexpr auto GAME_EXP_FACTOR = 8;

    State() {
        move_count = 0;
        std::fill(slots.begin(), slots.end(), 4);
        slots[P1_STORE] = 0;
        slots[P2_STORE] = 0;
    }

    // GETTERS
    auto get_turn() const -> int {
        return (move_count & 1) ? -1 : 1;
    }

    auto get_move_count() const -> int {
        return move_count;
    }

    auto& get_node() const {
        return slots;
    }

    // PREDICATES
    auto is_game_over() const -> bool {
        return std::all_of(
                   slots.begin() + 1,
                   slots.begin() + 7,
                   [](uint8_t x) { return x == 0; }) ||
               std::all_of(
                   slots.begin() + 8,
                   slots.begin() + 14,
                   [](uint8_t x) { return x == 0; });
    }

    auto is_legal(Move move) const -> bool {
        auto legals = legal_moves();
        return std::any_of(
            legals.begin(),
            legals.end(),
            [move](Move i) { return i == move; });
    }

    void set_node(unsigned long long xs, unsigned long long ys) {}

    void set_move_count(int n) {
        move_count = n;
    }

    // MOVE GENERATION
    auto num_legal_moves() const -> size_t {
        // this function is really hard to do for Kalah.
        return legal_moves().size();
    }
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////// UNDER CONSTRUCTION ////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    auto add_to_vec(std::vector<Move>& target) {
        
    }
    
    auto legal_moves() const -> std::vector<Move> {
        std::vector<Move> moves;
        bool done = false;
        // 1 or 8
        int offset = (move_count & 1) * 7 + 1;
        for (size_t i = offset; i < 6 + offset; i++) {
            if (slots[i] != 0) {
                moves.push_back(i - offset);
            }
        }
        // generate all legal moves
        // if a move ends in the pot, shift back and recurse.
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
        while (choice--) {
            // clear the least significant bit set
            bb &= bb - 1;
        }

        play(__builtin_ctz(bb));
    }

    // DATA VIEWS
    auto union_bitboard(int r) const -> Bitrow {
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

    auto pos_filled(int row, int col) const -> bool {
        // tests if a given location is filled, indexed by the row and column
        // this is done by indexing the row in the 2D array, then performing shifts
        // to produce a mask with only the desired bit. This mask is then AND-ed with
        // the row values to test for occupancy, as 0 = false and any other value =
        // true.
        return node[0][row] & (1 << col) || node[1][row] & (1 << col);
    }

    auto player_at(int row, int col) const -> bool {
        // only valid to use if posFilled returns true
        // this function essentially performs the same job as posFilled
        // except it only checks against the first half of the array
        // and assumes that the space is filled
        return node[0][row] & (1 << col);
        // true = X, false = O
    }

    auto probe_spot(int row, int col) const -> bool {
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
    auto horizontal_term() const -> int {
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

    auto vertical_term() const -> int {
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

    auto diagup_term() const -> int {
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

    auto diagdown_term() const -> int {
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

    auto evaluate() const -> int {
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

    auto heuristic_value() const -> int {
        int val = 0;
        for (int row = 0; row < NUM_ROWS; row++) {
            for (int i = 0; i < NUM_COLS; i++) {
                val += pos_filled(row, i) * (player_at(row, i) ? 1 : -1) * weights[i];
            }
        }
        return val;  // use some sort of central weighting approach
    }

    // DATA GENERATION
    auto game_running() const -> bool {
        // the game is over if the board is filled up or someone has 4-in-a-row
        return (!is_full() && evaluate() == 0);
    }

    auto game_result() const -> int {
        if (!is_game_over()) {
            std::cerr << "game_result() called on in-progress game.\n";
            return -2;
        }
        return evaluate();
    }

    auto get_position_contents(int row, int col) const -> int {
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

    auto vectorise_board() const {
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
        showvec(shiftedLegals);
    }

    auto get_player_move() const -> Move {
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
};

bool operator==(const State a, const State b) {
    return a.get_node()[0] == b.get_node()[0] && a.get_node()[1] == b.get_node()[1];
}
}  // namespace Kalah