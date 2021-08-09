#pragma once

#include <algorithm>
#include <array>
// #include <execution>
#include <vector>

#include "MCSearch.hpp"

// Possible heuristic improvement: use a long search to generate MCTS values for each starting square, use them as a heuristic starter.
// The RAVE approach makes this heuristic value = some sort of aggregate score of the move on parent nodes.
// UCT becomes (simulation value / rollouts) + (heuristic value / rollouts) + (exploration factor)

template <class State>
class Zero {
    using Move = typename State::Move;
    MCTS<State> search_driver = MCTS<State>();
    State node = State();
    static constexpr double epsilon = 0.1;

   public:
    Zero() {
        Zero(99);
    }
    Zero(const long long strength) {
        search_driver.set_time_limit(strength);
    }

    // SETTERS
    void set_time_limit(long long x) {
        search_driver.set_time_limit(x);
    }

    void set_rollout_limit(long long x) {
        search_driver.set_rollout_limit(x);
    }

    void set_readout(bool b) {
        search_driver.set_readout(b);
    }

    void set_debug(bool b) {
        search_driver.set_debug(b);
    }

    void set_node(State n) {
        node = n;
    }

    void use_time_limit(bool x) {
        search_driver.use_time_limit(x);
    }

    void use_rollout_limit(bool x) {
        search_driver.use_rollout_limit(x);
    }

    // GETTERS
    [[nodiscard]] auto get_player_move() const -> Move {
        return node.get_player_move();
    }

    [[nodiscard]] auto get_node() -> State& {
        return node;
    }

    [[nodiscard]] auto get_turn_modifier() const -> int {
        return node.get_turn();
    }

    [[nodiscard]] auto get_node_eval() const -> int {
        return node.evaluate();
    }

    [[nodiscard]] auto get_node_count() const {
        return search_driver.get_nodes();
    }

    [[nodiscard]] auto get_win_prediction() const -> double {
        // multiplies by 10 to get a weighted win-per-node percentage
        return 10 * search_driver.get_most_recent_winrate();
    }

    // PREDICATES
    [[nodiscard]] auto is_game_over() -> bool {
        return node.is_game_over();
    }

    // STATE INTERACTIONS
    void reset_node() {
        node.reset();
    }

    void engine_move() {
        search_driver.set_side(node.get_turn());
        node = search_driver.find_best_next_board(node);
    }

    [[nodiscard]] auto rollout_vector(State node) {
        std::vector<int> child_rollout_counts = search_driver.get_rollout_counts(node);
        std::vector<int> out(7);
        int idx = 0;
        for (int move : node.legal_moves()) {
            out[move] = child_rollout_counts[idx++];
        }
        return out;
    }

    [[nodiscard]] auto make_sample_move(const std::vector<int>& dist, State model) {
        int mod = std::reduce(dist.begin(), dist.end());
        // assert(mod != 0);
        int num = rand() % mod;
        for (auto i = 0; i < dist.size(); i++) {
            num -= dist[i];
            if (num <= 0) {
                model.play(i);
                break;
            }
        }
        return model;
    }

    [[nodiscard]] auto make_epsilon_greedy_move(const std::vector<int>& dist, State model) {
        double r = (double)rand() / (double)RAND_MAX;
        int move;
        if (r > epsilon) {
            move = std::distance(dist.begin(), std::max_element(dist.begin(), dist.end()));
        } else {
            move = rand() % dist.size();
        }
        model.play(move);
        return model;
    }

    // I/O
    void print(const std::string input, const std::string end = "\n") {
        std::cout << input << end;
    }

    void show_node() {
        node.show();
    }

    void show_result() const {
        // // assert(node.evaluate() == node.evaluateOLD());
        switch (node.evaluate()) {
            case 0:
                std::cout << "1/2-1/2" << '\n';
                break;
            case 1:
                std::cout << "1-0" << '\n';
                break;
            case -1:
                std::cout << "0-1" << '\n';
                break;
            default:
                std::cerr << "evaluate returned non-zero";
                break;
        }
    }
};