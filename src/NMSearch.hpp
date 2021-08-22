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
    static constexpr auto MAX_DEPTH = 64;

   private:
    using Move = typename State::Move;
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
    static constexpr auto PV_LEN = (MAX_DEPTH * MAX_DEPTH + MAX_DEPTH) / 2;
    std::array<Move, PV_LEN> pv_array;

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

    // SEARCH FUNCTIONS


    auto negamax(State &node, int depth, int colour, int a, int b) -> int {
        // assert(colour == 1 || colour == -1);
        // assert(depth >= 0);

        if (depth <= 0 || node.is_game_over()) {
            node_count++;
            return colour * (node.evaluate() * MATE_SCORE + node.heuristic_value());
        }
        int score;

        // // MAKE A NULL MOVE
        // node.pass_turn();
        // // PERFORM A LIMITED SEARCH
        // score = -negamax(node, depth - 3, -colour, -b, -a);
        // // UNMAKE NULL MOVE
        // node.unpass_turn();
        // a = std::max(a, score);
        // if (a >= b) {
        //     return a;
        // }

        for (auto move : node.legal_moves()) {
            node.play(move);
            score = -negamax(node, depth - 1, -colour, -b, -a);
            node.unplay(move);

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
        // assert(colour == 1 || colour == -1);

        if (node.is_game_over()) {
            node_count++;
            return colour * node.evaluate();
        }

        for (auto move : node.legal_moves()) {
            node.play(move);
            int score = -dnegamax(node, -colour, -b, -a);
            node.unplay(move);
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
        Move bestmove = 0; // valid but will always be changed by minimax
        int bestcase = N_INF;

        if constexpr (State::GAME_SOLVABLE) {
            unlimited_depth_minimax(node);
        } else {
            iterative_deepening_minimax(node);
        }
        show_search_result(bestmove, bestcase);
        node.play(bestmove);
        return node;
    }

    void show_search_result(Move bestmove, int bestcase) const {
        std::cout << "ISTUS:\n";
        std::cout << node_count << " nodes processed.\n";
        std::cout << "Best move found: " << bestmove << "\n";
        std::cout << "PV: ";
        // for (auto m : pv) std::cout << (int)(m + 1) << " ";
        std::cout << "\n";
        std::cout << "Istus win prediction: " << (int)((1 + bestcase) * (50)) << "%\n";
    }

    auto iterative_deepening_minimax(State &node) -> Move {
        auto end = std::chrono::steady_clock::now();
        end += std::chrono::milliseconds(time_limit);
        for (
            int depth = 1; 
            std::chrono::steady_clock::now() < end && depth < 22;
            depth++) {
            // principal_variation.clear();
            // principal_variation.reserve(256);
            auto start = std::chrono::steady_clock::now();
            int score = negamax(
                node, 
                depth, 
                node.get_turn(), 
                N_INF, 
                INF);
            std::cout << "depth: " << depth << " best move: " /*<< (int)principal_variation[0]*/ << " score: " << score << " in " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count() << "ms\n";
        }
        // return principal_variation[0];
        return 0;
    }

    auto unlimited_depth_minimax(State &node) -> Move {
        // principal_variation.clear();
        // principal_variation.reserve(256);
        dnegamax(node, node.get_turn());
        // return principal_variation[0];
        return 0;
    }

    void reset_nodes() {
        node_count = 0;
    }
};