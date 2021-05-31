#pragma once

#include <algorithm>
#include <array>
#include <iostream>
#include <numeric>
#include <vector>

namespace Connect4x4 {

class State {
   public:
    using Move = uint_fast8_t;
    using Bitrow = uint_fast8_t;
    static constexpr auto GAME_SOLVABLE = true;
    static constexpr auto NUM_ROWS = 4;
    static constexpr auto NUM_COLS = 4;
    static constexpr auto GAME_EXP_FACTOR = 8;
    static constexpr auto BB_ALL = 0b1111;
    static constexpr auto NUM_UNIQUE_MOVES = 4;
    static constexpr std::array<int, NUM_COLS> weights = {2, 1, 1, 2};

   private:
    std::array<std::array<Bitrow, NUM_COLS>, 2> node = {0};
    std::array<Move, NUM_ROWS * NUM_COLS> move_stack;
    int move_count = 0;

   public:
    State() {
        move_count = 0;
    }

    // GETTERS
    auto get_turn() const -> int {
        return (move_count & 1) ? -1 : 1;
    }

    auto get_move_count() const {
        return move_count;
    }

    auto get_turn_index() const {
        return move_count & 1;
    }

    auto get_node() const -> const std::array<std::array<Bitrow, NUM_COLS>, 2>& {
        return node;
    }  
    
    // PREDICATES
    auto is_full() const -> bool {
        return move_count == NUM_COLS * NUM_ROWS;
    }

    auto is_game_over() const -> bool {
        return is_full() || evaluate();
    }

    auto is_legal(Move move) const -> bool {
        auto legals = legal_moves();
        return std::any_of(
            legals.begin(),
            legals.end(),
            [move](Move i) { return i == move; });
    }

    void set_move_count(int n) {
        move_count = n;
    }

    // MOVE GENERATION
    auto num_legal_moves() const -> size_t {
        return NUM_COLS - __builtin_popcount(node[0][0] | node[1][0]);
    }

    auto legal_moves() const -> std::vector<Move> {
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
    auto union_bb(int r) const -> Bitrow {
        return node[0][r] | node[1][r];
    }

    auto pos_filled(int row, int col) const -> bool {
        return node[0][row] & (1 << col) || node[1][row] & (1 << col);
    }

    auto player_at(int row, int col) const -> bool  
    {
        return node[0][row] & (1 << col);
        //only valid to use if pos_filled() returns true, true = x, false = y
    }

    auto probe_spot(int row, int col) const -> bool {
        // tests the bit of the most recently played side
        return node[(move_count + 1) & 1][row] & (1 << col);
    }

    // STATE INTERACTIONS
    void mem_setup() {
        // move_stack.reserve(4 * 4);
    }

    inline void pass_turn() {
        move_count++;
    }

    void unpass_turn() {
        move_count--;
    }

    void reset() {
        std::fill(node[0].begin(), node[0].end(), 0);
        std::fill(node[1].begin(), node[1].end(), 0);
        std::fill(move_stack.begin(), move_stack.end(), 0);
        move_count = 0;
    }

    void play(int col) {
        // we assume play is not called on a filled column
        // iterate upward and break at the first empty position
        int row = NUM_ROWS;
        while (pos_filled(row - 1, col)) {
            row--;
        }
        // moveCount acts to determine which colour is played
        node[move_count & 1][row - 1] ^= (1 << col);
        // store the made move in the stack
        move_stack[move_count++] = col;
    }

    void unplay() {
        // decrement move counter and get the most recently played move
        int col = move_stack[--move_count];
        // iterate downward and break at the first filled position
        int row = 0;
        while (!pos_filled(row, col)) {
            row++;
        }
        // a bit is removed by XOR
        node[move_count & 1][row] ^= (1 << col);
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

    auto heuristic_value() -> int {
        int val = 0;
        for (int row = 0; row < NUM_ROWS; row++) {
            for (int i = 0; i < NUM_COLS; i++) {
                val += pos_filled(row, i) * (player_at(row, i) ? 1 : -1) * weights[i];
            }
        }
        return val;  // use some sort of central weighting approach
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

    void show_result() const 
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
};

bool operator==(State a, State b) {
    return !(a.get_node()[0] != b.get_node()[0] || a.get_node()[1] != b.get_node()[1]);
}
}  // namespace Connect4x4