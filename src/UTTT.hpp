#pragma once

#include <algorithm>
#include <array>
#include <iostream>
#include <numeric>
#include <vector>

#include "accelerations.hpp"

namespace MOD9 {
constexpr uint_fast8_t LOOKUP[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 0, 1, 2, 3, 4, 5, 6, 7, 8, 0, 1, 2, 3, 4, 5, 6, 7, 8, 0, 1, 2, 3, 4, 5, 6, 7, 8, 0, 1, 2, 3, 4, 5, 6, 7, 8, 0, 1, 2, 3, 4, 5, 6, 7, 8, 0, 1, 2, 3, 4, 5, 6, 7, 8, 0, 1, 2, 3, 4, 5, 6, 7, 8, 0, 1, 2, 3, 4, 5, 6, 7, 8};
}

namespace Board {

using Move = uint_fast8_t;
using Bitboard = uint_fast16_t;
using Num = int_fast8_t;

class SubState {
   public:
    Bitboard position[2] = {0b000000000, 0b000000000};
    // bool cached_death_state = false;

    void reset() {
        position[0] = 0b000000000;
        position[1] = 0b000000000;
    }

    inline auto union_bb() const -> Bitboard {
        return position[0] | position[1];
    }

    inline void play(const Num i, const Num idx) {
        // n ^ (1 << k) is a binary XOR where you flip the kth bit of n
        position[idx] ^= (1 << i);
    }

    inline void unplay(const Num prevmove, const Num idx)  // do not unplay on root
    {
        position[idx] ^= (1 << prevmove);
    }

    inline auto pos_filled(const Num i) const -> bool {
        return position[0] & (1 << i) || position[1] & (1 << i);
    }

    inline auto player_at(const Num i) const -> bool  //only valid to use if pos_filled() returns true, true = x, false = y
    {
        return (position[0] & (1 << i));
    }

    inline auto is_full() const -> bool {
        return position[0] + position[1] == 0b111111111;
    }

    inline auto evaluate() const -> Num {
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
        for (Num i = 0; i < 3; i++) {
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
        for (Num i = 0; i < 3; i++) {
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

    void show() const {
        for (Num x = 0; x < 3; x++) {
            for (Num y = 0; y < 3; y++) {
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

    inline auto is_board_dead() const -> bool {
        return is_full() || evaluate();
    }

    auto get_square_as_char(const Num square) const -> char {
        if (!pos_filled(square)) {
            return '.';
        } else {
            if (player_at(square)) {
                return 'X';
            } else {
                return 'O';
            }
        }
    }
};
}  // namespace Board

namespace UTTT {

constexpr auto GAME_SOLVABLE = false;
constexpr auto GAME_EXP_FACTOR = 1.41 * 5;

class State {
   public:
    using Move = uint_fast16_t;
    using Bitboard = uint_fast16_t;
    using Num = int_fast8_t;
    std::array<Board::SubState, 9> metaposition;
    Num forcingBoard = -1;
    Num movecount = 0;
    std::vector<Num> movestack;     // reserve for the playout boards
    std::vector<Num> forcingstack;  // reserve for the playout boards

    State() {
        movestack.reserve(16);
        forcingstack.reserve(16);
        for (Board::SubState &board : metaposition) {
            board = Board::SubState();
        }
        forcingstack.push_back(-1);
    }
    State(const UTTT::State &inputState) {
        metaposition = inputState.metaposition;
        forcingBoard = inputState.forcingBoard;
        movecount = inputState.movecount;
        forcingstack = inputState.forcingstack;
    }

    void show_debug() const {
        show();
        std::cout << "turn: " << (movecount & 1) << '\n';
        std::cout << "next forced board: " << forcingBoard << '\n';
        std::cout << "played moves: " << string(movestack) << '\n';
        std::cout << "previous forced boards: " << string(forcingstack) << '\n';
        Num counter = 0;
        for (const auto &substate : metaposition) {
            std::cout << "substate " << counter << ":\n";
            substate.show();
        }
    }

    inline void mem_setup() {
        movestack.reserve(81);
        forcingstack.reserve(81);
    }

    void reset() {
        std::for_each(
            metaposition.begin(),
            metaposition.end(),
            [](Board::SubState &b) { b.reset(); });
    }

    inline void play(const Num i) {
        const Num board = i / 9;
        const Num square = MOD9::LOOKUP[i];
        metaposition[board].play(square, movecount & 1);
        movestack.push_back(i);
        movecount++;
        forcingBoard = square;
        forcingstack.push_back(forcingBoard);
        assert(forcingstack.size() == movecount + 1);
    }

    inline void unplay()  // do not unplay on root
    {
        movecount--;
        const Num prevmove = movestack.back();
        const Num board = prevmove / 9;
        const Num square = MOD9::LOOKUP[prevmove];
        movestack.pop_back();
        metaposition[board].unplay(square, movecount & 1);
        forcingstack.pop_back();
        forcingBoard = forcingstack.back();
        assert(forcingstack.size() == movecount + 1);
    }

    inline auto board_won(const Num board) const -> bool {
        return metaposition[board].evaluate() != 0;
    }

    inline auto board_over(const Num board) const -> bool {
        return metaposition[board].is_board_dead();
    }

    inline auto winner_of_board(const Num board) const -> int_fast8_t  //only valid to use if pos_filled() returns true, true = x, false = y
    {
        return metaposition[board].evaluate();
    }

    inline auto is_full() -> bool {
        return std::all_of(metaposition.begin(), metaposition.end(), [](Board::SubState p) { return p.is_board_dead(); });
    }

    inline auto evaluate() const -> Num {
        // check first diagonal
        Num middle = winner_of_board(4);
        if (middle) {
            if (winner_of_board(0) == middle && middle == winner_of_board(8)) {
                return middle;
            }
            if (winner_of_board(2) == middle && middle == winner_of_board(6)) {
                return middle;
            }
        }

        // check second diagonal

        // check rows
        for (Num i = 0; i < 3; i++) {
            if (winner_of_board(i * 3)) {
                if (winner_of_board(i * 3) == winner_of_board(i * 3 + 1) && winner_of_board(i * 3 + 1) == winner_of_board(i * 3 + 2)) {
                    return winner_of_board(i * 3);
                }
            }
        }
        // check columns
        for (Num i = 0; i < 3; i++) {
            if (winner_of_board(i)) {
                if (winner_of_board(i) == winner_of_board(i + 3) && winner_of_board(i + 3) == winner_of_board(i + 6)) {
                    return winner_of_board(i);
                }
            }
        }
        Num count = 0;
        for (Num i = 0; i < 9; i++) {
            if (board_over(i)) {
                count += winner_of_board(i);
            } else {
                return 0;
            }
        }
        if (count > 0) {
            return 1;
        } else {
            return -1;
        }
    }

    inline void pass_turn() {
        movecount++;
    }

    auto get_turn() const -> uint_fast8_t {
        if (movecount & 1)
            return -1;
        else
            return 1;
    }

    inline auto is_game_over() const -> bool {
        if (num_legal_moves() == 0)
            return true;
        return (evaluate() != 0);
    }

    void show() const {
        for (Num x = 0; x < 3; x++) {
            for (Num y = 0; y < 3; y++) {
                if (board_over(x * 3 + y)) {
                    switch (winner_of_board(x * 3 + y)) {
                        case 1:
                            std::cout << "X ";
                            break;

                        case -1:
                            std::cout << "0 ";
                            break;

                        default:
                            break;
                    }
                } else {
                    std::cout << ". ";
                }
            }
            std::cout << "\n";
        }
        std::cout << "\n";
        Num board, square;
        const std::array<Num, 81> ordering = {
            0, 1, 2, 9, 10, 11, 18, 19, 20, 3, 4, 5, 12, 13, 14, 21, 22, 23, 6, 7, 8, 15, 16, 17, 24, 25, 26, 27, 28, 29, 36, 37, 38, 45, 46, 47, 30, 31, 32, 39, 40, 41, 48, 49, 50, 33, 34, 35, 42, 43, 44, 51, 52, 53, 54, 55, 56, 63, 64, 65, 72, 73, 74, 57, 58, 59, 66, 67, 68, 75, 76, 77, 60, 61, 62, 69, 70, 71, 78, 79, 80};
        Num counter = 0;
        std::string linebreak = " |-----------------------|\n";
        for (const auto &i : ordering) {
            board = i / 9;
            square = MOD9::LOOKUP[i];
            if (MOD9::LOOKUP[counter] == 0 && i != 0)
                std::cout << " |\n";
            if (i == 0 || i == 27 || i == 54)
                std::cout << linebreak;
            if (counter % 3 == 0)
                std::cout << " |";
            std::cout << ' ' << metaposition[board].get_square_as_char(square);
            counter++;
        }
        std::cout << " |\n";
        std::cout << linebreak << "\n\n";
    }

    auto num_legal_moves() const -> Num {
        if (forcingBoard != -1)
            return 9 - __builtin_popcount(metaposition[forcingBoard].union_bb());
        Num cnt = 0;
        for (Num i = 0; i < 9; i++) {
            if (!metaposition[i].is_board_dead())
                cnt += 9 - __builtin_popcount(metaposition[i].union_bb());
        }
        return cnt;
    }

    inline auto legal_moves() -> std::vector<Move> {
        std::vector<Move> moves;
        if (metaposition[forcingBoard].is_board_dead())
            forcingBoard = -1;
        if (forcingBoard == -1) {
            moves.reserve(81);
            Num bcounter = 0;
            for (const auto &board : metaposition) {
                if (!board.is_board_dead()) {
                    uint_fast16_t bb = ~board.union_bb() & 0b111111111;
                    while (bb) {
                        moves.push_back(bcounter * 9 + (__builtin_ctz(bb)));
                        bb &= bb - 1;  // clear the least significant bit set
                    }
                }
                bcounter++;
            }
        } else {
            moves.reserve(9);
            Bitboard bb = ~metaposition[forcingBoard].union_bb() & 0b111111111;
            while (bb) {
                moves.push_back(forcingBoard * 9 + (__builtin_ctz(bb)));
                bb &= bb - 1;  // clear the least significant bit set
            }
        }
        return moves;
    }

    inline void random_play() {
        std::vector<Move> moves = legal_moves();
        play(moves[rand() % moves.size()]);
    }

    inline auto heuristic_value() const -> uint_fast8_t {
        return rand() & 0b1111;
    }

    auto get_player_move() -> Move {
        const std::vector<Move> legals = legal_moves();
        std::vector<Move> shiftedLegals;
        std::transform(legals.begin(), legals.end(), std::back_inserter(shiftedLegals), [](Move n) { return n + 1; });
        std::cout << "Your legal moves are: ";
        showvec(shiftedLegals);
        std::cout << "\n--> ";
        int pos;
        std::cin >> pos;
        while (std::none_of(legals.begin(), legals.end(), [pos](Move m) { return m == (pos - 1); })) {
            std::cout << "invalid move.\n";
            show();
            std::cin >> pos;
        }
        return pos - 1;
    }
};

bool operator==(const Board::SubState &a, const Board::SubState &b) {
    return a.position == b.position;
}

bool operator==(const UTTT::State &a, const UTTT::State &b) {
    if (a.forcingBoard != b.forcingBoard)
        return false;
    for (short i = 0; i < 9; i++) {
        if (a.metaposition[i].position != b.metaposition[i].position)
            return false;
    }
    return true;
}
}  // namespace UTTT