#include <array>
#include <iostream>
#include <vector>

#include "accelerations.hpp"

namespace RawTree {

constexpr auto GAME_SOLVABLE = true;
constexpr auto gameexpfactor = 5;
const std::array<int, 12> NODE_VALUES = {8, 8, 7, 8, 5, 6, 7, 5, 8, 48, 1, 7};
const std::array<int, 12> CHILD_COUNT = {2, 1, 3, 2, 0, 2, 1, 0, 0, 0, 0, 0};
const std::array<int, 6> TERMINAL_NODES = {4, 7, 8, 9, 10, 11};
using Move = int_fast8_t;
using Bitboard = int_fast16_t;

class State {
   public:
    int node = 0;
    int_fast8_t turn = 1;
    std::vector<Move> movestack;

    void mem_setup() {
        movestack.reserve(4);
    }

    void reset() {
        node = 0;
    }

    void play(const Move i) {
        node = i;
        movestack.push_back(i);
        turn = -turn;
    }

    void unplay()  // do not unplay on root
    {
        Move prevmove = movestack.back();
        movestack.pop_back();
        node = prevmove;
        turn = -turn;
    }

    inline auto evaluate() const -> int_fast8_t {
        return NODE_VALUES[node];
    }

    void show() const {
        std::cout << node;
    }

    auto is_game_over() const -> bool {
        int current = node;
        return std::any_of(TERMINAL_NODES.begin(), TERMINAL_NODES.end(), [current](int n) { return current == n; });
    }

    auto num_legal_moves() const {
        return CHILD_COUNT[node];
    }

    auto get_turn() const {
        return 0;
    }

    auto legal_moves() const -> std::vector<Move> {
        std::vector<Move> moves;
        if (node == 0) {
            moves.push_back(1);
            moves.push_back(2);
        } else if (node == 1) {
            moves.push_back(3);
        } else if (node == 2) {
            moves.push_back(4);
            moves.push_back(5);
            moves.push_back(6);
        } else if (node == 3) {
            moves.push_back(7);
            moves.push_back(8);
        } else if (node == 5) {
            moves.push_back(9);
            moves.push_back(10);
        } else if (node == 6) {
            moves.push_back(11);
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
    return a.node == b.node;
}
}  // namespace RawTree