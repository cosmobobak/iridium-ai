#pragma once

#include <array>
#include <bitset>
#include <iostream>
#include <vector>

#include "accelerations.hpp"

namespace Gomoku {

constexpr auto GAME_SOLVABLE = false;
constexpr auto GAME_EXP_FACTOR = 1.41 * 5;
constexpr auto WIDTH = 8;
constexpr auto HEIGHT = 8;
constexpr std::array<char, 2> players = {'X', 'O'};

class State {
   public:
    using Move = uint_fast16_t;
    std::array<std::bitset<WIDTH * HEIGHT>, 2> node;
    uint_fast8_t turn = 1;
    std::vector<Move> movestack;

    inline void show() const {
        int row, col;
        for (row = 0; row < HEIGHT; ++row) {
            for (col = 0; col < WIDTH; ++col) {
                if (pos_filled(row * WIDTH + col)) {
                    if (player_at(row * WIDTH + col))
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

    inline void mem_setup() {
        movestack.reserve(WIDTH * HEIGHT);
    }

    inline auto get_turn() const -> uint_fast8_t {
        return turn;
    }

    inline auto player_at(const Move i) const -> bool  //only valid to use if pos_filled() returns true, true = x, false = y
    {
        return node[0].test(i);
    }

    inline auto player_at(const Move row, const Move col) const -> bool {
        return node[0].test(row * WIDTH + col);
    }

    inline auto pos_filled(const Move i) const -> bool {
        return (node[0] | node[1]).test(i);
    }

    inline auto pos_filled(const Move row, const Move col) const -> bool {
        return (node[0] | node[1]).test(row * WIDTH + col);
    }

    inline auto num_legal_moves() const -> uint_fast16_t {
        return WIDTH * HEIGHT - (node[0] | node[1]).count();
    }

    inline auto legal_moves() const -> std::vector<Move> {
        std::vector<Move> moves;
        moves.reserve(num_legal_moves());
        // moves.push_back(-1);
        for (Move square = 0; square < WIDTH * HEIGHT; square++) {
            if (!pos_filled(square)) {
                moves.push_back(square);
            }
        }
        return moves;
    }

    inline void random_play() {
        std::vector<Move> moves = legal_moves();
        play(moves[rand() % moves.size()]);
    }

    inline void pass_turn() {
        turn = -turn;
    }

    inline void play(Move square) {
        if (turn == 1) {
            node[0].set(square);
        } else {
            node[1].set(square);
        }
        turn = -turn;
        movestack.push_back(square);
    }

    inline void unplay() {
        Move prevmove = movestack.back();
        movestack.pop_back();
        if (turn == 1) {
            node[1].reset(prevmove);
        } else {
            node[0].reset(prevmove);
        }
        turn = -turn;
    }

    inline auto is_game_over() const -> bool {
        return num_legal_moves() == 0 || evaluate() != 0;
    }

    inline auto horizontal_term() const -> int_fast8_t {
        // iterates the starting positions of the rows
        for (Move row = 0; row < WIDTH * HEIGHT; row += WIDTH) {
            for (Move i = row; i < row + WIDTH - 4; i++) {
                if (pos_filled(i) &&
                    pos_filled(i + 1) &&
                    pos_filled(i + 2) &&
                    pos_filled(i + 3) &&
                    pos_filled(i + 4)) {
                    if (player_at(i) == player_at(i + 1) &&
                        player_at(i + 1) == player_at(i + 2) &&
                        player_at(i + 2) == player_at(i + 3) &&
                        player_at(i + 3) == player_at(i + 4)) {
                        if (player_at(i)) {
                            return 1;
                        } else {
                            return -1;
                        }
                    }
                }
            }
        }
        return 0;
    }

    inline auto vertical_term() const -> int_fast8_t {
        // iterates the starting positions of the columns
        for (Move col = 0; col < WIDTH; col++) {
            // this line below could be fucky
            for (Move i = col; i < col + (WIDTH * (HEIGHT - 4)); i += WIDTH) {
                if (pos_filled(i) &&
                    pos_filled(i + WIDTH * 1) &&
                    pos_filled(i + WIDTH * 2) &&
                    pos_filled(i + WIDTH * 3) &&
                    pos_filled(i + WIDTH * 4)) {
                    if (player_at(i) == player_at(i + WIDTH * 1) &&
                        player_at(i + WIDTH * 1) == player_at(i + WIDTH * 2) &&
                        player_at(i + WIDTH * 2) == player_at(i + WIDTH * 3) &&
                        player_at(i + WIDTH * 3) == player_at(i + WIDTH * 4)) {
                        if (player_at(i)) {
                            return 1;
                        } else {
                            return -1;
                        }
                    }
                }
            }
        }
        return 0;
    }

    inline auto diagdown_term() const -> int_fast8_t {
        // iterates the starting positions of the rows
        for (Move row = 0; row < HEIGHT - 4; row++) {
            for (Move col = 0; col < WIDTH - 4; col++) {
                if (pos_filled(row, col) &&
                    pos_filled(row + 1, col + 1) &&
                    pos_filled(row + 2, col + 2) &&
                    pos_filled(row + 3, col + 3) &&
                    pos_filled(row + 4, col + 4)) {
                    if (player_at(row, col) == player_at(row + 1, col + 1) &&
                        player_at(row + 1, col + 1) == player_at(row + 2, col + 2) &&
                        player_at(row + 2, col + 2) == player_at(row + 3, col + 3) &&
                        player_at(row + 3, col + 3) == player_at(row + 4, col + 4)) {
                        if (player_at(row, col)) {
                            return 1;
                        } else {
                            return -1;
                        }
                    }
                }
            }
        }
        return 0;
    }

    inline auto diagup_term() const -> int_fast8_t {
        // iterates the starting positions of the rows
        for (Move row = 4; row < HEIGHT; row++) {
            for (Move col = 0; col < WIDTH - 4; col++) {
                if (pos_filled(row, col) &&
                    pos_filled(row - 1, col + 1) &&
                    pos_filled(row - 2, col + 2) &&
                    pos_filled(row - 3, col + 3) &&
                    pos_filled(row - 4, col + 4)) {
                    if (player_at(row, col) == player_at(row - 1, col + 1) &&
                        player_at(row - 1, col + 1) == player_at(row - 2, col + 2) &&
                        player_at(row - 2, col + 2) == player_at(row - 3, col + 3) &&
                        player_at(row - 3, col + 3) == player_at(row - 4, col + 4)) {
                        if (player_at(row, col)) {
                            return 1;
                        } else {
                            return -1;
                        }
                    }
                }
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

    auto get_player_move() -> Move {
        const std::vector<Move> legals = legal_moves();
        std::vector<Move> shiftedLegals;
        std::transform(legals.begin(), legals.end(), std::back_inserter(shiftedLegals), [](Move n) { return n + 1; });
        std::vector<Move> rows, cols;
        for (auto m : legals)
        {
            rows.push_back(m / WIDTH + 1);
            cols.push_back(m % WIDTH + 1);
        }
        
        std::cout << "Your legal moves are: " << zipstring(rows, cols) << "\n";
        Move row, col, pos;
        std::cout << "Enter row: ";
        std::cin >> row;
        std::cout << "Enter col: ";
        std::cin >> col;
        pos = (row - 1) * WIDTH + (col - 1);
        while (std::none_of(legals.begin(), legals.end(), [pos](Move m) { return m == (pos); })) {
            std::cout << "invalid move.\n";
            std::cout << "Enter row: ";
            std::cin >> row;
            std::cout << "Enter col: ";
            std::cin >> col;
            pos = (row - 1) * WIDTH + (col - 1);
        }
        return pos;
    }

    auto heuristic_value() const -> uint_fast8_t {
        return 0;
    }
};

bool operator==(const State a, const State b) {
    if (a.turn != b.turn)
        return false;
    if (a.node != b.node)
        return false;
    return true;
}
}  // namespace Gomoku