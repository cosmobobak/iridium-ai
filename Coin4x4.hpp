#include <array>
#include <iostream>
#include <vector>

#include "accelerations.hpp"

namespace Coin4x4 {

constexpr auto GAME_SOLVABLE = true;
constexpr auto gameexpfactor = 5;
constexpr std::array<int_fast8_t, 4> weights = {2, 1, 1, 2};
constexpr std::array<int_fast8_t, 2> players = {-1, 1};
using Move = int_fast8_t;
using Bitboard = int_fast8_t;

class State {
   public:
    std::array<std::array<Bitboard, 4>, 2> bbnode = {0};
    int_fast8_t turn = 1;
    int_fast32_t nodes = 0;
    std::vector<Move> movestack;

    State() {
        movestack.reserve(16);
    }

    void mem_setup() {
        movestack.reserve(4 * 4);
    }

    auto union_bb(int_fast8_t r) const -> Bitboard {
        return bbnode[0][r] | bbnode[1][r];
    }

    auto is_full() const -> bool  //WORKING
    {
        return union_bb(0) == 0b1111;
    }

    void show() const  //WORKING
    {
        int_fast8_t row, col;
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

    auto pos_filled(int_fast8_t row, int_fast8_t col) const -> bool {
        return bbnode[0][row] & (1 << col) || bbnode[1][row] & (1 << col);
    }

    auto pos_filled(int_fast8_t col) const -> bool {
        return bbnode[0][0] & (1 << col) || bbnode[1][0] & (1 << col);
    }

    auto player_at(int_fast8_t row, int_fast8_t col) const -> bool  //only valid to use if pos_filled() returns true, true = x, false = y
    {
        return (bbnode[0][row] & (1 << col));
    }

    auto num_legal_moves() const -> int_fast8_t {
        return 4 - popcount(bbnode[0][0] | bbnode[1][0]);
    }

    auto legal_moves() const -> std::vector<Move> {
        std::vector<Move> moves;
        moves.reserve(4);
        int_fast8_t bb = ~(bbnode[0][0] | bbnode[1][0]) & 0b1111;
        for (; bb;) {
            moves.push_back(lsb(bb));
            bb &= bb - 1;  // clear the least significant bit set
        }
        return moves;
    }

    void random_play() {
        std::vector<Move> moves = legal_moves();
        play(moves[rand() % moves.size()]);
    }

    inline void pass_turn() {
        turn = -turn;
    }

    void play(Move col)  //WORKING
    {
        for (int_fast8_t row = 4; row; row--) {
            if (!pos_filled(row - 1, col)) {
                if (turn == 1)
                    bbnode[0][row - 1] |= (1 << col);
                else
                    bbnode[1][row - 1] |= (1 << col);
                break;
            }
        }
        pass_turn();
        movestack.push_back(col);
    }

    void unplay()  //WORKING
    {
        Move col = movestack.back();
        movestack.pop_back();
        for (int_fast8_t row = 0; row < 4; row++) {
            if (pos_filled(row, col)) {
                if (turn == 1)
                    bbnode[1][row] &= ~(1 << col);
                else
                    bbnode[0][row] &= ~(1 << col);
                break;
            }
        }
        pass_turn();
    }

    auto horizontal_term() const -> int_fast8_t {
        for (int_fast8_t row = 0; row < 4; row++) {
            const int_fast8_t col = 0;
            if (pos_filled(row, col) &&
                pos_filled(row, col + 1) &&
                pos_filled(row, col + 2) &&
                pos_filled(row, col + 3)) {
                if (player_at(row, col) == player_at(row, col + 1) &&
                    player_at(row, col + 1) == player_at(row, col + 2) &&
                    player_at(row, col + 2) == player_at(row, col + 3)) {
                    if (player_at(row, col))
                        return 1;
                    else
                        return -1;
                }
            }
        }
        return 0;
    }

    auto vertical_term() const -> int_fast8_t {
        const int_fast8_t row = 0;
        for (int_fast8_t col = 0; col < 4; col++) {
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
        return 0;
    }

    auto diagup_term() const -> int_fast8_t {
        const int_fast8_t row = 3;
        const int_fast8_t col = 0;
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
        return 0;
    }

    auto diagdown_term() const -> int_fast8_t {
        const int_fast8_t row = 0;
        const int_fast8_t col = 0;
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
        return 0;
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
        if (evaluate() != 0 || is_full() == true) {
            return true;
        } else {
            return false;
        }
    }

    auto heuristic_value() -> int_fast8_t {
        int_fast8_t val = 0;
        for (int_fast8_t row = 0; row < 4; row++) {
            for (int_fast8_t i = 0; i < 4; i++) {
                val += pos_filled(row, i) * (player_at(row, i) ? 1 : -1) * weights[i];
            }
        }
        return -(val * 10);  // use some sort of central weighting approach
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
    if (a.turn != b.turn)
        return false;
    if (a.bbnode[0] != b.bbnode[0] || a.bbnode[1] != b.bbnode[1])
        return false;
    return a.movestack == b.movestack;
}
}  // namespace Coin