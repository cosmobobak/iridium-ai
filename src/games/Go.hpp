#pragma once

#include <array>
#include <iostream>
#include <vector>
#include <bitset>

namespace Go {

constexpr auto WIDTH = 7;
constexpr auto HEIGHT = 7;
constexpr std::array<char, 2> players = {'X', 'O'};

class State {
   public:
    using Move = int;
    using Bitboard = unsigned long long;
    std::array<std::bitset<WIDTH * HEIGHT>, 2> node;
    static constexpr auto NUM_UNIQUE_MOVES = WIDTH * HEIGHT;
    int turn = 1;
    std::vector<Move> movestack;

    void show() const {
        int row, col;
        for (row = 0; row < HEIGHT; ++row) {
            for (col = 0; col < WIDTH; ++col) {
                if (pos_filled(row * 3 + col)) {
                    if (player_at(row * 3 + col))
                        std::cout << "X ";
                    else
                        std::cout << "0 ";
                } else
                    std::cout << ". ";
            }
            std::cout << '\n';
        }
        std::cout << '\n';
    }

    auto player_at(const uint_fast8_t i) const -> bool  //only valid to use if pos_filled() returns true, true = x, false = y
    {
        return node[0].test(i);
    }

    auto pos_filled(const uint_fast8_t i) const -> bool {
        return (node[0] | node[1]).test(i);
    }

    auto num_legal_moves() const -> size_t {
        return WIDTH * HEIGHT - (node[0] | node[1]).count();
    }

    auto legal_moves() const -> std::vector<int> {
        std::vector<Move> moves;
        moves.reserve(num_legal_moves());
        moves.push_back(-1);
        for (int square = 0; square < WIDTH * HEIGHT; square++) {
            if (!pos_filled(square)) {
                moves.push_back(square);
            }
        }
        return moves;
    }

    void random_play() {
        std::vector<Move> moves = legal_moves();
        play(moves[rand() % moves.size()]);
    }

    void pass_turn() {
        turn = -turn;
    }

    void play(Move square) {
        if (square != -1) {
            // turn passed
            node[square] = turn;
        }
        turn = -turn;
        movestack.push_back(square);
    }

    void unplay() {
        int square = movestack.back();
        node[square] = 0;
        turn = -turn;
        movestack.pop_back();
    }

    auto is_game_over() -> int {
        return ((movestack.end() - 1) == (movestack.end() - 2) && movestack.back() == -1);
    }

    void show_result() {
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
}  // namespace Go