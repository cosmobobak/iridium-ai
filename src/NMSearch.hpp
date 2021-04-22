#pragma once

#include <chrono>
#include <limits>
#include <vector>

template <class State>
class Negamax {
   public:
    static constexpr auto MATE_SCORE = 100000;
    static constexpr auto INF = std::numeric_limits<int>::max();
    static constexpr auto N_INF = std::numeric_limits<int>::lowest() + 1;

   private:
    using typename State::Move;
    using Line = std::vector<Move>;
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
    Line principal_variation;

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
    auto get_nodes() const -> int {
        return node_count;
    }

    auto negamax(State &node, int depth, int colour, int a, int b, Line& principal_variation) -> int {
        assert(colour == 1 || colour == -1);
        assert(depth >= 0);

        Line current_line;

        if (depth == 0) {
            node_count++;
            return colour * (node.evaluate() * MATE_SCORE + node.heuristic_value());
        }

        if (node.is_game_over()) {
            node_count++;
            return colour * node.evaluate() * MATE_SCORE * (depth + 1);
        }

        int score;
        for (auto move : node.legal_moves()) {
            node.play(move);
            score = -negamax(node, depth - 1, -colour, -b, -a, &current_line);
            node.unplay();

            if (score >= b) {
                // beta cutoff
                return b;
            }
            if (score > a) {
                // move that raises alpha
                a = score;
            }
        }
        return a;
    }

    auto dnegamax(State &node, int colour, int a = N_INF, int b = INF) -> int {
        assert(colour == 1 || colour == -1);

        if (node.is_game_over()) {
            node_count++;
            return colour * node.evaluate();
        }

        for (auto move : node.legal_moves()) {
            node.play(move);
            int score = -dnegamax(node, -colour, -b, -a);
            node.unplay();
            // std::cout << "score for move " << (int)move << ": " << score << "\n";

            if (score >= b) {
                return b;
            }
            a = std::max(a, score);
        }

        return a;
    }

    auto find_best_next_board(State node) -> State {
        reset_nodes();
        Move bestmove = -1;
        int bestcase = N_INF;

        Line principal_variation;
        if (State::GAME_SOLVABLE) {
            unlimited_depth_minimax(node, bestmove, bestcase);
        } else {
            iterative_deepening_minimax(node, bestmove, bestcase, principal_variation);
        }
        show_search_result(bestmove, bestcase, principal_variation);
        node.play(bestmove);
        return node;
    }

    void show_search_result(Move bestmove, int bestcase, const Line &principal_variation) const {
        std::cout << "ISTUS:\n";
        std::cout << node_count << " nodes processed.\n";
        std::cout << "Best move found: " << bestmove << "\n";
        std::cout << "PV: ";
        for (int i = 0; i < principal_variation.length; i++) std::cout << (int)(principal_variation.moves[i] + 1) << " ";
        std::cout << "\n";
        std::cout << "Istus win prediction: " << (int)((1 + bestcase) * (50)) << "%\n";
    }

    void iterative_deepening_minimax(State &node, Move &bestmove, int &bestcase, Line &principal_variation) {
        auto end = std::chrono::steady_clock::now();
        end += std::chrono::milliseconds(time_limit);
        int depth = 1;
        while (std::chrono::steady_clock::now() < end && depth < 22) {
            auto start = std::chrono::steady_clock::now();
            for (auto move : node.legal_moves()) {
                node.play(move);
                int score = -negamax(node, depth, node.get_turn(), N_INF, INF, &principal_variation);
                node.unplay();
                if (bestcase < score) {
                    bestcase = score;
                    bestmove = move;
                }
            }
            std::cout << "depth: " << depth << " best move: " << (int)bestmove << " score: " << bestcase << " in " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count() << "ms\n";
            depth++;
        }
    }

    void unlimited_depth_minimax(State &node, int &bestmove, int &bestcase) {
        for (auto move : node.legal_moves()) {
            node.play(move);
            int score = -dnegamax(node, node.get_turn());
            node.unplay();
            std::cout << "score for move " << (int)move << ": " << score << "\n";
            if (bestcase < score) {
                bestcase = score;
                bestmove = move;
            }
        }
    }

    void reset_nodes() {
        node_count = 0;
    }
};