#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <iostream>
#include <numeric>
#include <sstream>
#include <vector>

#include "../utilities/rng.hpp"

#define popcnt __builtin_popcount

constexpr std::array masks = {
    0b111000000,
    0b000111000,
    0b000000111,
    0b100100100,
    0b010010010,
    0b001001001,
    0b100010001,
    0b001010100};

constexpr auto contains_mask(int bb) -> bool {
    for (auto m : masks) {
        if ((bb & m) == m) {
            return true;
        }
    }
    return false;
}

namespace UTTT {
class Square3x3 {
   public:
    std::array<int16_t, 2> slots = {0};

    auto is_won() const -> bool {
        for (auto mask : masks) {
            if ((slots[0] & mask) == mask) {
                return true;
            }
            if ((slots[1] & mask) == mask) {
                return true;
            }
        }
        return false;
    }

    template < int player >
    auto is_won_by() const -> bool {
        for (auto mask : masks) {
            if ((slots[player] & mask) == mask)
                return true;
        }
        return false;
    }

    auto is_dead() const -> bool {
        return is_won() || (slots[0] | slots[1]) == 0b111111111;
    }

    auto filled_slot_count() const -> int {
        return popcnt(slots[0] | slots[1]);
    }

    friend auto operator==(const Square3x3& a, const Square3x3& b) -> bool {
        return a.slots[0] == b.slots[0] && a.slots[1] == b.slots[1];
    }
};

class State {
   public:
    static constexpr auto GAME_SOLVABLE = false;
    static constexpr auto GAME_EXP_FACTOR = 8;
    static constexpr auto NUM_ACTIONS = 81;
    static constexpr auto MAX_GAME_LENGTH = 81;
    using Move = int;

   private:
    static constexpr auto NO_SQUARE = -1;

    // the game has nine sub-games, each of which is a 3x3 grid
    std::array<Square3x3, 9> node;
    // a cache of which sub-games have ended that can be changed inside const methods
    mutable std::array<bool, 9> square_ended_cache = {false};
    // the number of moves made so far
    int move_count;
    // the square upon which the player to move must play
    int current_forced_square;
    // the forcing square on the turn before this one
    int last_forced_square;
    // whether the last move requires a game-over check
    bool change_flag = true;
    // the last result of check_game_over()
    bool last_gameover_val = false;

   public:
    State() {
        std::fill(node.begin(), node.end(), Square3x3());
        move_count = 0;
        current_forced_square = NO_SQUARE;
        last_forced_square = NO_SQUARE;
    }

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

    auto get_node() const -> const std::array<Square3x3, 9>& {
        return node;
    }

    template < int player >
    auto get_global_bitmask() const -> int {
        int binary_accumulator = 0;
        for (const auto& sq : node) {
            binary_accumulator <<= 1;
            binary_accumulator |= sq.is_won_by<player>();
        }
        return binary_accumulator;
    }

    // SETTERS
    void set_move_count(int n) {
        move_count = n;
    }

    // PREDICATES
   private:
    auto cached_is_dead(int sq_idx) const -> bool {
        // NOTE FOR FUTURE COSMO: YOU HAVE USED THE mutable KEYWORD, HERE BE DRAGONS.
        if (square_ended_cache[sq_idx]) {
            return true;
        }
        auto death_status = node[sq_idx].is_dead();
        square_ended_cache[sq_idx] = death_status;
        return death_status;
    }

    auto check_game_over() const -> bool {
        for (auto sq_idx = 0; sq_idx < 9; ++sq_idx) {
            if (cached_is_dead(sq_idx)) {
                return true;
            }
        }

        return contains_mask(get_global_bitmask<0>()) 
            || contains_mask(get_global_bitmask<1>());
    }
   public:
    auto is_game_over() -> bool {
        if (move_count == MAX_GAME_LENGTH) {
            return true;
        }
        // check if we need to do a check at all
        if (!change_flag)
            return last_gameover_val;

        // reset the change flag
        change_flag = false;
        last_gameover_val = check_game_over();

        return last_gameover_val;
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
        if (current_forced_square == NO_SQUARE) {
            int count = 81;
            for (int sq_idx = 0; sq_idx < 9; ++sq_idx) {
                if (!cached_is_dead(sq_idx)) {
                    count -= node[sq_idx].filled_slot_count();
                } else {
                    count -= 9;
                }
            }
            return count;
        } else {
            return 9 - node[current_forced_square].filled_slot_count();
        }
    }

    auto legal_moves() const -> std::vector<Move> {
        std::vector<Move> moves(num_legal_moves());
        int i = 0;
        if (current_forced_square == NO_SQUARE) {
            for (int sq_idx = 0; sq_idx < 9; ++sq_idx) {
                if (!cached_is_dead(sq_idx)) {
                    int bb = ~union_bb(node[sq_idx]) & 0b111111111;
                    while (bb) {
                        moves[i++] = sq_idx * 9 + __builtin_ctz(bb);
                        bb &= bb - 1;  // clear the least significant bit set
                    }
                }
            }
        } else {
            int bb = ~union_bb(node[current_forced_square]) & 0b111111111;
            while (bb) {
                moves[i++] = current_forced_square * 9 + __builtin_ctz(bb);
                bb &= bb - 1;  // clear the least significant bit set
            }
        }
        return moves;
    }

    void random_play() {
        assert(num_legal_moves() > 0);
        int choice = rng::random_int(num_legal_moves());
        if (current_forced_square == NO_SQUARE) {
            for (int sq_idx = 0; sq_idx < 9; ++sq_idx) {
                if (!cached_is_dead(sq_idx)) {
                    int bb = ~union_bb(node[sq_idx]) & 0b111111111;
                    while (bb) {
                        if (!choice--) {
                            play(sq_idx * 9 + __builtin_ctz(bb));
                            return;
                        }
                        bb &= bb - 1;  // clear the least significant bit set
                    }
                }
            }
        } else {
            int bb = ~union_bb(node[current_forced_square]) & 0b111111111;
            while (bb) {
                if (!choice--) {
                    play(current_forced_square * 9 + __builtin_ctz(bb));
                    return;
                }
                bb &= bb - 1;  // clear the least significant bit set
            }
        }
    }

    // DATA VIEWS
    inline static auto union_bb(const Square3x3& square) -> int {
        return square.slots[0] | square.slots[1];
    }

    // STATE INTERACTIONS
    void mem_setup() {
    }

    void pass_turn() {
        move_count++;
    }

    void unpass_turn() {
        move_count--;
    }

    void reset() {
        std::fill(node.begin(), node.end(), Square3x3());
        move_count = 0;
        current_forced_square = NO_SQUARE;
        last_forced_square = NO_SQUARE;
    }

    void play(int n) {
        // calculate parameters for modification
        int target_square = n / 9;
        int location_in_square = n % 9;
        // add a bit to the square
        node[target_square].slots[move_count & 1] ^= 1 << location_in_square;
        // as we are moving forward, the current forced square becomes the last forced square
        last_forced_square = current_forced_square;
        // set the new forced square
        if (cached_is_dead(location_in_square)) {
            // if the target is unplayable, the forced square is NO_SQUARE
            current_forced_square = NO_SQUARE;
        } else {
            // else the forced square is now the location in the played square.
            current_forced_square = location_in_square;
        }
        ++move_count;

        // if this move won a square, we need be checking wins
        if (cached_is_dead(target_square))
            change_flag = true;
    }

    void unplay(int n) {
        move_count--;
        // calculate parameters for modification
        int target_square = n / 9;
        int location_in_square = n % 9;
        // remove a bit from the square
        node[target_square].slots[move_count & 1] ^= 1 << location_in_square;
        // as we are moving backward, the last forced square becomes the current forced square
        current_forced_square = last_forced_square;
        // you'd think we now need to set LFS to something else, but we
        // A. do not have access to that information
        // B. do not ever need to access LFS again, as we never unplay more than one move at a time.

        last_gameover_val = false;
    }

    // EVALUATION
    auto evaluate() const -> int {
        // construct two binary numbers that represent the meta-board in the same way that the squares individually operate
        int xs = get_global_bitmask<0>();
        if (contains_mask(xs))
            return 1;

        int os = get_global_bitmask<1>();
        if (contains_mask(os)) 
            return -1;
        
        int diff = popcnt(xs) - popcnt(os);

        // return the sign of diff using boolean arithmetic
        return (diff > 0) - (diff < 0);
    }

    auto heuristic_value() {
        return 0;
    }

    // IO
    void show() const {
        auto evaluate_square = [](const Square3x3& sq) {
            int xs = sq.slots[0];
            int os = sq.slots[1];
            if (contains_mask(xs)) {
                return 1;
            } else if (contains_mask(os)) {
                return -1;
            } else {
                return 0;
            }
        };

        std::stringstream sb;
        for (int x = 0; x < 3; x++) {
            for (int y = 0; y < 3; y++) {
                switch (evaluate_square(node[x * 3 + y])) {
                    case 1:
                        sb << "X ";
                        break;

                    case -1:
                        sb << "0 ";
                        break;

                    case 0:
                        sb << (node[x * 3 + y].is_dead() ? "D " : ". ");
                        break;

                    default:
                        sb << ":(";
                        break;
                }
            }
            sb << "\n";
        }
        sb << "\n";

        int board, square;
        static constexpr std::array<int, 81> ordering = {
            0, 1, 2, 9, 10, 11, 18, 19, 20, 3, 4, 5, 12, 13, 14, 21, 22, 23, 6, 7, 8, 15, 16, 17, 24, 25, 26, 27, 28, 29, 36, 37, 38, 45, 46, 47, 30, 31, 32, 39, 40, 41, 48, 49, 50, 33, 34, 35, 42, 43, 44, 51, 52, 53, 54, 55, 56, 63, 64, 65, 72, 73, 74, 57, 58, 59, 66, 67, 68, 75, 76, 77, 60, 61, 62, 69, 70, 71, 78, 79, 80};
        int counter = 0;
        std::string linebreak = " ├───────┼───────┼───────┤\n";
        std::string top = "┌───────┬───────┬───────┐\n";
        std::string bottom = "└───────┴───────┴───────┘\n";
        for (const auto& i : ordering) {
            board = i / 9;
            square = i % 9;
            if (counter % 9 == 0 && i != 0)
                sb << " │\n";
            if (i == 0 || i == 27 || i == 54)
                sb << linebreak;
            if (counter % 3 == 0)
                sb << " │";
            sb << ' ' << (union_bb(node[board]) & (1 << square) ? (node[board].slots[0] & (1 << square) ? 'X' : '0') : '.');
            counter++;
        }
        sb << " │\n";
        sb << linebreak << "\n\n";

        std::cout << sb.str();
        // std::cout << "\ncurrent forced board: " << current_forced_square << "\n";
        // std::cout << "last forced board: " << last_forced_square << "\n";
        // std::cout << "eval: " << evaluate() << "\n";
        // std::cout << "num legal moves: " << num_legal_moves() << "\n";
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

    friend auto operator==(const State& a, const State& b) -> bool {
        return a.move_count == b.move_count && a.current_forced_square == b.current_forced_square && a.node == b.node;
    }
};
}  // namespace UTTT