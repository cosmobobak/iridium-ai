#include <iostream>
#include <vector>
#include <array>

#include "accelerations.hpp"

namespace TicTacToe {

constexpr auto GAME_SOLVABLE = true;
constexpr auto gameexpfactor = 5;
using Move = int_fast8_t;
using Bitboard = int_fast16_t;

class State {
   public:
    std::array<Bitboard, 2> position = {0b000000000, 0b000000000};
    int_fast8_t turn = 1;
    std::vector<Move> movestack;

    void mem_setup() {
        movestack.reserve(9);
    }

    void reset() {
        position[0] = 0b000000000;
        position[1] = 0b000000000;
    }

    void play(const Move i) {
        // n ^ (1 << k) is a binary XOR where you flip the kth bit of n
        if (turn == 1)
            position[0] |= (1 << i);
        else
            position[1] |= (1 << i);
        turn = -turn;
        movestack.push_back(i);
    }

    void unplay()  // do not unplay on root
    {
        Move prevmove = movestack.back();
        movestack.pop_back();
        if (turn == 1)
            position[1] &= ~(1 << prevmove);
        else
            position[0] &= ~(1 << prevmove);
        turn = -turn;
    }

    auto pos_filled(const int_fast8_t i) const -> bool {
        if (((position[0] | position[1]) & (1L << i)) != 0)
            return true;
        else
            return false;
    }

    auto player_at(const int_fast8_t i) const -> bool  //only valid to use if pos_filled() returns true, true = x, false = y
    {
        if ((position[0] & (1L << i)) != 0)
            return true;
        else
            return false;
    }

    auto is_full() const -> bool {
        for (int_fast8_t i = 0; i < 9; i++) {
            if (!pos_filled(i))
                return false;
        }
        return true;
    }

    inline auto evaluatex() const -> int_fast8_t {
        const Bitboard x = position[0];
        const Bitboard o = position[1];
        if ((x & 0b111000000) == 0b111000000)  // highrow
            return 1;
        if ((x & 0b000111000) == 0b000111000)  // midrow
            return 1;
        if ((x & 0b000000111) == 0b000000111)  // lowrow
            return 1;
        if ((x & 0b100010001) == 0b100010001)  // diaglrdown
            return 1;
        if ((x & 0b001010100) == 0b001010100)  // diaglrup
            return 1;
        if ((x & 0b100100100) == 0b100100100)  // lcol
            return 1;
        if ((x & 0b010010010) == 0b010010010)  // mcol
            return 1;
        if ((x & 0b001001001) == 0b001001001)  // rcol
            return 1;
        if ((o & 0b111000000) == 0b111000000)  // highrow
            return -1;
        if ((o & 0b000111000) == 0b000111000)  // midrow
            return -1;
        if ((o & 0b000000111) == 0b000000111)  // lowrow
            return -1;
        if ((o & 0b100010001) == 0b100010001)  // diaglrdown
            return -1;
        if ((o & 0b001010100) == 0b001010100)  // diaglrup
            return -1;
        if ((o & 0b100100100) == 0b100100100)  // lcol
            return -1;
        if ((o & 0b010010010) == 0b010010010)  // mcol
            return -1;
        if ((o & 0b001001001) == 0b001001001)  // rcol
            return -1;
        return 0;
    }

    inline auto evaluate() const -> int_fast8_t {
        // check first diagonal
        if (pos_filled(0) && pos_filled(4) && pos_filled(8)) {
            if (player_at(0) == player_at(4) && player_at(4) == player_at(8)) {
                if (player_at(0))
                    return 1;
                else
                    return -1;
            }
        }
        // check second diagonal
        if (pos_filled(2) && pos_filled(4) && pos_filled(6)) {
            if (player_at(2) == player_at(4) && player_at(4) == player_at(6)) {
                if (player_at(2))
                    return 1;
                else
                    return -1;
            }
        }
        // check rows
        for (int_fast8_t i = 0; i < 3; i++) {
            if (pos_filled(i * 3) && pos_filled(i * 3 + 1) && pos_filled(i * 3 + 2)) {
                if (player_at(i * 3) == player_at(i * 3 + 1) && player_at(i * 3 + 1) == player_at(i * 3 + 2)) {
                    if (player_at(i * 3))
                        return 1;
                    else
                        return -1;
                }
            }
        }
        // check columns
        for (int_fast8_t i = 0; i < 3; i++) {
            if (pos_filled(i) && pos_filled(i + 3) && pos_filled(i + 6)) {
                if (player_at(i) == player_at(i + 3) && player_at(i + 3) == player_at(i + 6)) {
                    if (player_at(i))
                        return 1;
                    else
                        return -1;
                }
            }
        }
        return 0;
    }

    void pass_turn() {
        turn = -turn;
    }

    void show() const {
        for (int_fast8_t x = 0; x < 3; x++) {
            for (int_fast8_t y = 0; y < 3; y++) {
                if (pos_filled(x * 3 + y)) {
                    if (player_at(x * 3 + y))
                        std::cout << "X ";
                    else
                        std::cout << "0 ";
                } else
                    std::cout << ". ";
            }
            std::cout << "\n";
        }
        std::cout << "\n";
    }

    auto is_game_over() const -> bool {
        return (evaluate() != 0) || is_full();
    }

    auto num_legal_moves() const {
        return 9 - popcount(position[0] | position[1]);
    }

    auto legal_moves() const -> std::vector<Move> {
        std::vector<Move> moves;
        moves.reserve(9);
        Bitboard bb = ~(position[0] | position[1]) & 0b111111111;
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

    auto heuristic_value() const -> int_fast8_t {
        return 0;
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

bool operator==(const State a, const State b) {
    if (a.turn != b.turn)
        return false;
    if (a.position != b.position)
        return false;
    return a.movestack == b.movestack;
}
}  // namespace Glyph