#pragma once

#include <chrono>
#include <limits>

#include "TreeNode.hpp"
#include "UCT.hpp"

using TreeNode::Node;

template <class State, int UCT_EXP_FACTOR>
class MCTS {
    static constexpr int REWARD = 10;
    // limiter on search time
    long long time_limit;
    // limiter on rollouts
    long long rollout_limit;

    // the perspective that we're searching from
    int side;

    // flags
    bool readout = true;
    bool debug = true;
    bool limit_by_rollouts;
    bool limit_by_time;

    // recorded search data
    // the win / loss ratio of the most recently played move
    double last_winloss;
    int node_count;

    // dictates whether we preserve a part of the tree across moves
    // bool memsafe = true;
    // Node<State>* preservedNode = nullptr;

   public:
    MCTS() {
        MCTS(1, 3);
    }
    MCTS(int player, long long strength) {
        srand(time(NULL));
        time_limit = strength;
        side = player;
        limit_by_rollouts = false;
        limit_by_time = true;
        last_winloss = 5;
        node_count = 0;
    }
    MCTS(int player, long long strength, bool limiter_type) {
        srand(time(NULL));
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
        srand(time(NULL));
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
    auto get_nodes() const -> int_fast32_t {
        return node_count;
    }

    double get_most_recent_winrate() const {
        return last_winloss;
    }

    // auto prune(Node<State>* parent, const State& target) -> Node<State>* {
    //     Node<State>* out = nullptr;
    //     bool found = false;
    //     for (Node<State>* child : parent->children) {
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

        Node<State>* root_node = new Node<State>(board);
        root_node->set_state(board);
        root_node->set_player_no(-side);

        do {
            select_expand_simulate_backpropagate(root_node);
            node_count++;
            show_debug(root_node);
        } while (
            (!limit_by_time || std::chrono::steady_clock::now() < end) 
            && (!limit_by_rollouts || node_count < rollout_limit));

        State out = root_node->best_child()->get_state();
        
        if (readout) {
            auto time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();
            std::cout << node_count << " nodes processed in " << time << "ms at " << (double)node_count / ((double)time / 1000.0) << "NPS.\n";
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

        Node<State>* root_node = new Node<State>(board);
        root_node->set_state(board);
        root_node->set_player_no(-side);

        do {
            select_expand_simulate_backpropagate(root_node);
            node_count++;
        } while ((!limit_by_time || std::chrono::steady_clock::now() < end) && (!limit_by_rollouts || node_count < rollout_limit));

        if (readout) {
            auto time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();
            std::cout << node_count << " nodes processed in " << time << "ms at " << (double)node_count / ((double)time / 1000.0) << "NPS.\n";
        }

        last_winloss = root_node->best_child()->get_winrate();
        last_winloss = std::max(last_winloss, 0.0);

        int len = root_node->get_children().size();
        std::vector<int> out(len);
        for (int child_idx = 0; child_idx < len; child_idx++) {
            out[child_idx] = root_node->get_children()[child_idx]->get_visit_count();
        }

        deleteTree(root_node);
        return out;
    }

    inline void select_expand_simulate_backpropagate(Node<State>* root_node) {
        int result;
        // SELECTION
        Node<State>* promisingNode = select_promising_node(root_node);

        // EXPANSION
        if (!promisingNode->board.is_game_over())
            promisingNode->expand();

        Node<State>* nodeToExplore = promisingNode;

        if (promisingNode->children.size() != 0)
            nodeToExplore = promisingNode->random_child();

        // SIMULATION
        result = simulate_playout(nodeToExplore);
        // BACKPROPAGATION
        backprop(nodeToExplore, result);
    }

    inline auto select_promising_node(Node<State>* root_node) const -> Node<State>* {
        Node<State>* node = root_node;
        while (node->children.size())
            node = UCT<Node<State>, UCT_EXP_FACTOR>::best_node_uct(node);
        return node;
    }

    inline void backprop(Node<State>* nodeToExplore, int winner) {
        Node<State>* propagator = nodeToExplore;
        while (propagator) {
            propagator->increment_visits();
            if (propagator->get_player_no() == winner) {
                propagator->add_score(REWARD);
            }
            propagator = propagator->get_parent();
        }
    }

    inline auto simulate_playout(Node<State>* node) -> int {
        State playout_board = node->copy_state();
        playout_board.mem_setup();

        // tests for an immediate loss in the position 
        // and sets to MIN_VALUE if there is one.
        int status = playout_board.evaluate();
        if (status == -side) {
            node->get_parent()->set_win_score(std::numeric_limits<int>::lowest());
            return status;
        }

        // play out until game over
        while (!playout_board.is_game_over()) {
            playout_board.random_play();
        }
        
        return playout_board.evaluate();
    }

    // DEBUG
    void show_debug(Node<State>* root_node) const {
        if (debug && (node_count & 0b111111111111111) == 0b111111111111111) {
            root_node->show_child_visitrates();
            std::cout << "| ";
            root_node->show_child_winrates();
            std::cout << "| ";
            for (auto child : root_node->get_children()) {
                std::cout << UCT<Node<State>, UCT_EXP_FACTOR>::compute_uct(child) << " ";
            }
            std::cout << "\n";
        }
    }
};