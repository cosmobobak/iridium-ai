#pragma once

#include <chrono>

template <class State>
class Negamax {
    // limiter on search time
    long long time_limit;
    // limiter on depth
    long long depth_limit;
    // TT transpositionTable;

    // flags
    bool readout = true;
    bool debug = true;
    bool limit_by_depth;
    bool limit_by_time;

    // recorded search data
    int node_count;

   public:
    Negamax() {
        Negamax(99);
    }
    Negamax(long long strength) {
        time_limit = strength;
        limit_by_depth = false;
        limit_by_time = true;
        node_count = 0;
    }

    void use_depth_limit(bool x) {
        limit_by_depth = x;
        limit_by_time = !x;
    }

    void use_time_limit(bool x) {
        limit_by_time = x;
        limit_by_depth = !x;
    }

    // SETTERS
    void set_time_limit(long long tl) {
        time_limit = tl;
    }

    void set_readout(bool b) {
        readout = b;
    }

    void set_debug(bool b) {
        debug = b;
    }

    void set_depth_limit(long long dl) {
        depth_limit = dl;
    }

    // GETTERS
    auto get_nodes() -> int {
        return node_count;
    }

    auto negamax(State& node, int depth = 10, int colour = 1, int a = -20000, int b = 20000) -> int {
        assert(colour != 0);

        if (depth < 1) {
            node_count++;
            return colour * node.evaluate();
        }

        if (node.is_game_over()) {
            node_count++;
            return colour * node.evaluate() * (depth + 1);
        }

        int score;

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

    auto dnegamax(State& node, int colour = 1, int a = -20000, int b = 20000) -> int {
        if (node.is_game_over()) {
            node_count++;
            return colour * node.evaluate();
        }

        int score;

        for (auto move : node.legal_moves()) {
            node.play(move);
            node_count += 1;
            score = -dnegamax(node, -colour, -b, -a);
            node.unplay();

            if (score >= b)
                return b;
            score = std::max(a, score);
        }

        return a;
    }

    auto find_best_next_board(State node) -> State {
        reset_nodes();
        int bestmove = -1;
        int bestcase = -40;
        int score = -40;

        auto end = std::chrono::steady_clock::now();
        end += std::chrono::milliseconds(time_limit);

        if (false) {
            //StateType::GAME_SOLVABLE
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
            int depth = 1;
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
        std::cout << node_count << " nodes processed.\n";
        std::cout << "Istus win prediction: " << (int)((1 + bestcase) * (50)) << "%\n";
        node.play(bestmove);
        return node;
    }

    void reset_nodes() {
        node_count = 0;
    }
};