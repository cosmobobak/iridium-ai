#pragma once

#include <algorithm>
#include <array>
#include <iostream>
#include <numeric>
#include <sstream>
#include <vector>

namespace TicTacToe {

class State {
   public:
    using Move = uint_fast8_t;
    using Bitboard = uint_fast16_t;

   private:
    std::array<Bitboard, 2> node = {0};
    std::array<Move, 9> move_stack;
    int move_count = 0;

   public:
    static constexpr auto GAME_SOLVABLE = true;
    static constexpr auto GAME_EXP_FACTOR = 6;
    static constexpr auto MAX_GAME_LENGTH = 9;
    static constexpr auto NUM_ACTIONS = 9;
    static constexpr auto BB_ALL = 0b111111111;

    // GETTERS
    auto get_turn() const -> int {
        return (move_count & 1) ? -1 : 1;
    }

    auto get_move_count() const -> int {
        return move_count;
    }

    auto get_turn_index() const -> int {
        return move_count & 1;
    }

    auto get_node() const -> const std::array<Bitboard, 2>& {
        return node;
    }

    // PREDICATES
    auto is_full() const -> bool {
        return move_count == 9;
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

    // MOVE GENERATION
    auto num_legal_moves() const -> size_t {
        return 9 - __builtin_popcount(node[0] | node[1]);
    }

    auto legal_moves() const -> std::vector<Move> {
        Bitboard bb = node[0] | node[1];
        std::vector<Move> moves(9 - __builtin_popcount(bb));
        bb = ~bb & BB_ALL;
        int counter = 0;
        while (bb) {
            moves[counter++] = __builtin_ctz(bb);
            bb &= bb - 1;  // clear the least significant bit set
        }
        return moves;
    }

    void random_play() {
        // the union of the top rows
        Bitboard bb = node[0] | node[1];
        int num_moves = 9 - __builtin_popcount(bb);

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
    auto pos_filled(const int i) const -> bool {
        return (node[0] | node[1]) & (1 << i);
    }

    auto player_at(const int i) const -> bool {
        return node[0] & (1 << i);
        //only valid to use if pos_filled() returns true, true = x, false = y
    }

    auto probe_spot(int i) const -> bool {
        // tests the bit of the most recently played side
        return node[(move_count + 1) & 1] & (1 << i);
    }

    // STATE INTERACTIONS
    void mem_setup() {
        // move_stack.reserve(9);
    }

    void pass_turn() {
        move_count++;
    }

    void unpass_turn() {
        move_count--;
    }

    void reset() {
        std::fill(node.begin(), node.end(), 0);
    }

    void play(int i) {
        node[move_count & 1] |= (1 << i);
        move_stack[move_count++] = i;
    }

    void set_move_count(int n) {
        move_count = n;
    }

    void unplay() {
        int i = move_stack[--move_count];
        node[move_count & 1] ^= (1 << i);
    }

    void unplay(int i) {
        move_count--;
        node[move_count & 1] ^= (1 << i);
    }

    // EVALUATION
    auto evaluate() const -> int {
        // check first diagonal
        if (probe_spot(0) && probe_spot(4) && probe_spot(8)) {
            return -get_turn();
        }
        // check second diagonal
        if (probe_spot(2) && probe_spot(4) && probe_spot(6)) {
            return -get_turn();
        }
        // check rows
        for (int i = 0; i < 3; i++) {
            if (probe_spot(i * 3) && probe_spot(i * 3 + 1) && probe_spot(i * 3 + 2)) {
                return -get_turn();
            }
        }
        // check columns
        for (int i = 0; i < 3; i++) {
            if (probe_spot(i) && probe_spot(i + 3) && probe_spot(i + 6)) {
                return -get_turn();
            }
        }
        return 0;
    }

    auto heuristic_value() const -> int {
        return 0;
    }

    // I/O
    auto charat(int y, int x) const {
        if (pos_filled(x * 3 + y)) {
            if (player_at(x * 3 + y))
                return 'X';
            else
                return '0';
        } else {
            return ' ';
        }
    }

    void show() const {
        std::stringstream sb;
        sb << "  " << charat(0, 0) << " │ " << charat(1, 0) << " │ " << charat(2, 0) << " \n";
        sb << " ───┼───┼─── \n";
        sb << "  " << charat(0, 1) << " │ " << charat(1, 1) << " │ " << charat(2, 1) << " \n";
        sb << " ───┼───┼─── \n";
        sb << "  " << charat(0, 2) << " │ " << charat(1, 2) << " │ " << charat(2, 2) << " \n";
        std::cout << sb.str();
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
};

bool operator==(const State a, const State b) {
    return a.get_node() == b.get_node();
}
}  // namespace TicTacToe