#pragma once

#include <iostream>
#include <vector>
#include <array>

#include "accelerations.hpp"

namespace TicTacToe {

constexpr auto GAME_SOLVABLE = true;
constexpr auto GAME_EXP_FACTOR = 1.41 * 5;

class State {
   public:
    using Move = uint_fast8_t;
    using Bitboard = uint_fast16_t;
    std::array<Bitboard, 2> position = {0b000000000, 0b000000000};
    uint_fast8_t movecount = 0;
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
        position[movecount & 1] |= (1 << i);
        movecount++;
        movestack.push_back(i);
    }

    void unplay()  // do not unplay on root
    {
        movecount--;
        Move prevmove = pop(movestack);
        position[movecount & 1] &= ~(1 << prevmove);
    }

    auto pos_filled(const uint_fast8_t i) const -> bool {
        return (position[0] | position[1]) & (1 << i);
    }

    auto player_at(const uint_fast8_t i) const -> bool  //only valid to use if pos_filled() returns true, true = x, false = y
    {
        if ((position[0] & (1L << i)) != 0)
            return true;
        else
            return false;
    }

    auto is_full() const -> bool {
        for (uint_fast8_t i = 0; i < 9; i++) {
            if (!pos_filled(i))
                return false;
        }
        return true;
    }

    auto is_legal(Move move) const -> bool {
        auto legals = legal_moves();
        return std::any_of(
            legals.begin(),
            legals.end(),
            [move](Move i) { return i == move; });
    }

    auto evaluate() const -> int_fast8_t {
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
        for (int i = 0; i < 3; i++) {
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
        for (int i = 0; i < 3; i++) {
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
        movecount++;
    }

    auto get_turn() const -> int_fast8_t {
        if (movecount & 1)
            return -1;
        else
            return 1;
    }

    void show() const {
        for (int x = 0; x < 3; x++) {
            for (int y = 0; y < 3; y++) {
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
        return 9 - __builtin_popcount(position[0] | position[1]);
    }

    auto legal_moves() const -> std::vector<Move> {
        std::vector<Move> moves;
        moves.reserve(9);
        Bitboard bb = ~(position[0] | position[1]) & 0b111111111;
        for (; bb;) {
            moves.push_back(__builtin_ctz(bb));
            bb &= bb - 1;  // clear the least significant bit set
        }
        return moves;
    }

    void random_play() {
        std::vector<Move> moves = legal_moves();
        play(moves[rand() % moves.size()]);
    }

    auto heuristic_value() const -> uint_fast8_t {
        return 0;
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
    return a.position == b.position;
}
}  // namespace Glyph