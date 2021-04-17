#pragma once

#include <chrono>

namespace SearchDriver {

template <class StateType>
class Negamax {
    uint_fast32_t nodes;
    long long timeLimit;
    // TT transpositionTable;

   public:
    Negamax() {
        Negamax(99);
    }
    Negamax(const long long TL) {
        timeLimit = TL;
    }

    void set_time_limit(long long tl) {
        timeLimit = tl;
    }

    void set_nodes(int_fast32_t n) {
        nodes = n;
    }

    auto get_nodes() -> int_fast32_t {
        return nodes;
    }

    auto negamax(
        StateType& node,
        uint_fast8_t depth = 10,
        uint_fast8_t colour = 1,
        int_fast16_t a = -20000,
        int_fast16_t b = 20000) -> int_fast16_t  //WORKING
    {
        assert(colour != 0);

        if (depth < 1) {
            nodes++;
            return colour * node.evaluate();
        }

        if (node.is_game_over()) {
            nodes++;
            return colour * node.evaluate() * (depth + 1);
        }

        int_fast16_t score;

        for (auto move : node.legal_moves()) {
            node.play(move);
            score = -negamax(node, depth - 1, -colour, -b, -a);
            node.unplay();

            if (score >= b)
                return b;
            if (score > a)
                a = score;
        }
        return a;
    }

    auto dnegamax(
        StateType& node,
        uint_fast8_t colour = 1,
        int_fast16_t a = -20000,
        int_fast16_t b = 20000) -> int_fast16_t  //WORKING
    {
        if (node.is_game_over()) {
            nodes++;
            return colour * node.evaluate();
        }
        int_fast16_t score;

        for (auto move : node.legal_moves()) {
            node.play(move);
            nodes += 1;
            score = -dnegamax(node, -colour, -b, -a);
            node.unplay();

            if (score >= b)
                return b;
            if (score > a)
                a = score;
        }

        return a;
    }

    auto find_best_next_board(StateType node) -> StateType {
        reset_nodes();
        auto bestmove = -1;
        int_fast16_t bestcase = -40;
        int_fast16_t score = -40;
        nodes = 0;

        auto end = std::chrono::steady_clock::now();
        end += std::chrono::milliseconds(timeLimit);

        if (false) {  //StateType::GAME_SOLVABLE
            for (const auto& move : node.legal_moves()) {
                node.play(move);
                score = -dnegamax(node, node.get_turn());
                node.unplay();
                if (bestcase < score) {
                    bestcase = score;
                    bestmove = move;
                }
            }
        } else {
            uint_fast8_t depth = 1;
            while (std::chrono::steady_clock::now() < end && depth < 22) {
                auto start = std::chrono::steady_clock::now();
                for (const auto move : node.legal_moves()) {
                    node.play(move);
                    score = -negamax(node, depth, -node.get_turn());
                    node.unplay();
                    if (bestcase < score) {
                        bestcase = score;
                        bestmove = move;
                    }
                }
                std::cout << "depth: " << (int)depth << " best move: " << (int)bestmove << " score: " << (int)bestcase << " in " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count() << "ms\n";
                depth++;
            }
        }
        std::cout << "ISTUS:\n";
        std::cout << nodes << " nodes processed.\n";
        std::cout << "Istus win prediction: " << (int)((1 + bestcase) * (50)) << "%\n";
        node.play(bestmove);
        return node;
    }

    void reset_nodes() {
        nodes = 0;
    }
};
}  // namespace SearchDriver

namespace NMSearch {

template <class StateType>
class Istus {
   public:
    SearchDriver::Negamax<StateType> searchDriver = SearchDriver::Negamax<StateType>();
    StateType node = StateType();

    Istus() {
        Istus(99);
    }
    Istus(const long long strength) {
        searchDriver.set_time_limit(strength);
    }

    inline void print(const std::string input, const std::string end = "\n") {
        std::cout << input << end;
    }

    auto get_player_move() {
        return node.get_player_move();
    }

    auto get_node() {
        return node;
    }

    void engine_move() {
        node = searchDriver.find_best_next_board(node);
    }

    void show_result() const {
        int r;
        r = node.evaluate();
        if (r == 0)
            std::cout << "1/2-1/2" << '\n';
        else if (r == 1)
            std::cout << "1-0" << '\n';
        else
            std::cout << "0-1" << '\n';
    }
};
}  // namespace NMSearch