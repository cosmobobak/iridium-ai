#pragma once

#include <chrono>
#include <limits>

#include "TreeNode.hpp"
#include "UCT.hpp"

using TreeNode::Node;

namespace SearchDriver {
constexpr int REWARD = 10;

template <class State, int UCT_EXP_FACTOR>
class MCTS {
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

    void set_nodes(int_fast32_t n) {
        node_count = n;
    }

    auto get_nodes() const -> int_fast32_t {
        return node_count;
    }

    // GETTERS
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
        int status = playout_board.evaluate();
        if (status == -side) {
            // tests for an immediate loss in the position and sets to MIN_VALUE
            // if there is one.
            node->get_parent()->set_win_score(std::numeric_limits<int>::lowest());
            return status;
        }
        while (!playout_board.is_game_over()) {
            playout_board.random_play();
        }
        status = playout_board.evaluate();
        return status;
    }
};
} // namespace SearchDriver

// Possible heuristic improvement: use a long search to generate MCTS values for each starting square, use them as a heuristic starter.
// The RAVE approach makes this heuristic value = some sort of aggregate score of the move on parent nodes.
// UCT becomes (simulation value / rollouts) + (heuristic value / rollouts) + (exploration factor)

namespace MCSearch {

template <class State, int EXP_FACTOR>
class Zero {
    SearchDriver::MCTS<State, EXP_FACTOR> search_driver = SearchDriver::MCTS<State, EXP_FACTOR>();
    State node = State();

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
    auto get_player_move() const {
        return node.get_player_move();
    }

    auto get_node() -> State& {
        return node;
    }

    auto get_turn_modifier() const {
        return node.get_turn();
    }

    auto get_node_eval() const {
        return node.evaluate();
    }

    auto get_node_count() const {
        return search_driver.get_nodes();
    }

    auto get_win_prediction() const -> double {  
        // multiplies by 10 to get a weighted win-per-node percentage
        return 10 * search_driver.get_most_recent_winrate();
    }

    // PREDICATES
    auto is_game_over() const -> bool {
        return node.is_game_over();
    }

    // STATE INTERACTIONS
    void reset_node() {
        node.reset();
    }

    void engine_move() {
        node = search_driver.find_best_next_board(node);
    }

    auto rollout_vector(State node) {
        std::vector<int> child_rollout_counts = search_driver.get_rollout_counts(node);
        std::vector<int> out(7);
        int idx = 0;
        for (int move : node.legal_moves()) {
            out[move] = child_rollout_counts[idx++];
        }
        return out;
    }

    auto make_sample_move(std::vector<int> dist, State model) {
        int mod = std::accumulate(dist.begin(), dist.end(), 0);
        assert(mod != 0);
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

    // I/O
    void print(const std::string input, const std::string end = "\n") {
        std::cout << input << end;
    }
    
    void show_node() {
        node.show();
    }

    void show_result() const {
        // assert(node.evaluate() == node.evaluateOLD());
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
}  // namespace MCSearch