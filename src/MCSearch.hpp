#pragma once

#include <chrono>
#include <limits>

#include "UCT.hpp"
#include "TreeNode.hpp"

constexpr auto INF = std::numeric_limits<int>::max();
constexpr auto N_INF = std::numeric_limits<int>::lowest() + 1;

template <class State>
class MCTS {
   private:
    using Node = TreeNode::TreeNode<State>;
    static constexpr auto MAX_REWARD = 10;
    // limiter on search time
    long long time_limit;
    // limiter on rollouts
    long long rollout_limit;

    // the perspective that we're searching from
    int side;

    // memory slot for playouts, to reduce allocation
    // State playout_board;

    // flags
    bool readout = true;
    bool debug = false;
    bool limit_by_rollouts;
    bool limit_by_time;

    // recorded search data
    // std::array<int, State::NUM_UNIQUE_MOVES> amaf_counters;
    // the win / loss ratio of the most recently played move
    double last_winloss;
    int node_count;

    // dictates whether we preserve a part of the tree across moves
    // bool memsafe = true;
    // Node* preservedNode = nullptr;

   public:
    MCTS() {
        MCTS(1, 3);
    }
    MCTS(int player, long long strength) {
        srand(time(nullptr));
        time_limit = strength;
        side = player;
        limit_by_rollouts = false;
        limit_by_time = true;
        last_winloss = 5;
        node_count = 0;
    }
    MCTS(int player, long long strength, bool limiter_type) {
        srand(time(nullptr));
        this->limit_by_rollouts = limiter_type;
        this->limit_by_time = !limiter_type;
        if (limiter_type) {
            rollout_limit = strength;
        } else {
            time_limit = strength;
        }
        side = player;
        last_winloss = 5;
        node_count = 0;
    }
    MCTS(bool url) {
        srand(time(nullptr));
        time_limit = 1000;
        side = 1;
        limit_by_rollouts = url;
        limit_by_time = !url;
        last_winloss = 5;
        node_count = 0;
    }

    void use_rollout_limit(bool x) {
        limit_by_rollouts = x;
        limit_by_time = !x;
    }

    void use_time_limit(bool x) {
        limit_by_time = x;
        limit_by_rollouts = !x;
    }

    // SETTERS
    void set_time_limit(long long tl) {
        time_limit = tl;
    }

    void set_opponent(int i) {
        side = -i;
    }

    void set_side(int i) {
        side = i;
    }

    void set_readout(bool b) {
        readout = b;
    }

    void set_debug(bool b) {
        debug = b;
    }

    void set_rollout_limit(long long rl) {
        rollout_limit = rl;
    }

    // GETTERS
    [[nodiscard]] auto get_nodes() const -> int {
        return node_count;
    }

    [[nodiscard]] auto get_most_recent_winrate() const -> double {
        return last_winloss;
    }

    // auto prune(Node* parent, const State& target) -> Node* {
    //     Node* out = nullptr;
    //     bool found = false;
    //     for (Node* child : parent->children) {
    //         if (!found && child->get_state() == target) {
    //             out = child;
    //             found = true;
    //         } else
    //             delete child;
    //     }
    //     if (out)
    //         out->set_parent(nullptr);
    //     delete parent; // FIX THIS
    //     return out;
    // }

    auto find_best_next_board(const State board) -> State {
        // board is immediately copied    ^^^

        node_count = 0;
        side = board.get_turn();

        // tracks time
        auto start = std::chrono::steady_clock::now();
        auto end = start + std::chrono::milliseconds(time_limit);

        Node* root_node = new Node(board);
        root_node->set_state(board);
        root_node->set_player_no(-side);

        // assert(limit_by_rollouts != limit_by_time);
        do {
            select_expand_simulate_backpropagate(root_node);
            node_count++;
            show_debug(root_node);
        } while (
            (!limit_by_time || std::chrono::steady_clock::now() < end) && (!limit_by_rollouts || node_count < rollout_limit));

        State out = root_node->best_child()->get_state();

        if (readout) {
            auto time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();
            std::cout << node_count << " nodes processed in " << time << "ms at " << (double)node_count / ((double)time / 1000.0) << "NPS.\n";
            std::cout << "predicted winrate: " << root_node->best_child()->get_winrate() << "\n";
        }

        delete root_node;
        return out;
    }

    auto get_rollout_counts(const State board) -> std::vector<int> {
        // board is immediately copied  ^^^

        node_count = 0;
        side = board.get_turn();

        // tracks time
        auto start = std::chrono::steady_clock::now();
        auto end = start + std::chrono::milliseconds(time_limit);

        Node* root_node = new Node(board);
        root_node->set_state(board);
        root_node->set_player_no(-side);

        // assert(limit_by_rollouts != limit_by_time);
        do {
            select_expand_simulate_backpropagate(root_node);
            node_count++;
            show_debug(root_node);
        } while (
            (!limit_by_time || std::chrono::steady_clock::now() < end) && (!limit_by_rollouts || node_count < rollout_limit));

        if (readout) {
            auto time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();
            std::cout << node_count << " nodes processed in " << time << "ms at " << (double)node_count / ((double)time / 1000.0) << "NPS.\n";
            std::cout << "predicted winrate: " << root_node->best_child()->get_winrate() << "\n";
        }

        last_winloss = root_node->best_child()->get_winrate();
        last_winloss = std::max(last_winloss, 0.0);

        int len = root_node->get_children().size();
        std::vector<int> out(len);
        for (int child_idx = 0; child_idx < len; child_idx++) {
            out[child_idx] = root_node->get_children()[child_idx]->get_visit_count();
        }

        delete root_node;
        return out;
    }

    void select_expand_simulate_backpropagate(Node* root_node) {
        // SELECTION
        Node* promisingNode = select_promising_node(root_node);

        // EXPANSION
        if (!promisingNode->get_state().is_game_over()) {
            promisingNode->expand();
        }

        Node* nodeToExplore = promisingNode;

        if (promisingNode->get_children().size() != 0) {
            nodeToExplore = promisingNode->random_child();
        }

        // SIMULATION
        int result = simulate_playout(nodeToExplore);
        // BACKPROPAGATION
        backprop(nodeToExplore, result);
    }

    auto select_promising_node(Node* root_node) const -> Node* {
        Node* node = root_node;
        while (node->get_children().size()) {
            node = UCT<Node, State::GAME_EXP_FACTOR>::best_child_ucb1(node);
        }
        return node;
    }

    [[nodiscard]] auto relative_reward(int perspective, int reward) const -> int {
        // designed for two-player zero-sum environments.
        // my win == your loss
        if (perspective == this->side) {
            return reward;
        } else {
            return MAX_REWARD - reward;
        }
    }

    void backprop(Node* nodeToExplore, int reward) {
        // works its way up the tree, adding relative scores to all the parent nodes.
        for (Node* bp_node = nodeToExplore; bp_node != nullptr; bp_node = bp_node->get_parent()) {
            bp_node->increment_visits();
            bp_node->add_score(relative_reward(bp_node->get_player_no(), reward));
        }
    }

    auto simulate_playout(Node* node) -> int {
        State playout_board = node->copy_state();
        playout_board.mem_setup();

        // tests for an immediate loss in the position
        // and sets to MIN_VALUE if there is one.
        int status = playout_board.evaluate();
        if (status == -side) {
            node->get_parent()->set_win_score(N_INF);
            return status;
        }

        // play out until game over
        while (!playout_board.is_game_over()) {
            playout_board.random_play();
        }

        return (playout_board.evaluate() + 1) * 5;  // 1/0/-1 -> 10/5/0
        // return playout_board.evaluate() == side ? 10 : -10;  // 1/0/-1 -> 10/0/-10
    }

    // DEBUG
    void show_debug(Node* root_node) const {
        if (debug && (node_count & 0b111111111111111) == 0b111111111111111) {
            root_node->show_child_visitrates();
            std::cout << "| ";
            root_node->show_child_winrates();
            std::cout << "| ";
            for (auto child : root_node->get_children()) {
                std::cout << UCT<Node, State::GAME_EXP_FACTOR>::compute_ucb1(child) << " ";
            }
            std::cout << "| ";
            double lw = root_node->best_child()->get_winrate();
            lw = std::max(lw, 0.0);
            std::cout << (root_node->get_state().get_turn() == 1 ? 10 * lw : 100 - 10 * lw);
            std::cout << "\n";
        }
    }
};