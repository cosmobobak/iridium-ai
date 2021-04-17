#pragma once

#include <chrono>

#include "TreeNode.hpp"
#include "UCT.hpp"

using namespace TreeNode;

namespace SearchDriver {
constexpr uint_fast8_t REWARD = 10;

template <class StateType, int UCT_EXP_FACTOR = 6>
class MCTS {
    // limiter on search time
    long long time_limit;
    // limiter on rollouts
    long long rollout_limit;

    // the win score that the opponent wants
    uint_fast8_t opponent;

    // flags
    bool readout = true;
    bool use_rollout_limit;
    bool use_time_limit;

    // recorded search data
    // the win / loss ratio of the most recently played move
    double last_winloss;
    int node_count;

    // dictates whether we preserve a part of the tree across moves
    const bool memsafe = true;
    Node<StateType>* preservedNode = nullptr;

   public:
    MCTS() {
        MCTS(1, 3);
    }
    MCTS(const int_fast8_t player, const long long strength) {
        srand(time(NULL));
        time_limit = strength;
        opponent = -player;
        use_rollout_limit = false;
        use_time_limit = true;
        last_winloss = 5;
        node_count = 0;
    }
    MCTS(const int_fast8_t player, const long long strength, bool use_rollout_limit) {
        srand(time(NULL));
        this->use_rollout_limit = use_rollout_limit;
        this->use_time_limit = !use_rollout_limit;
        if (use_rollout_limit) {
            rollout_limit = strength;
        } else {
            time_limit = strength;
        }
        opponent = -player;
        last_winloss = 5;
        node_count = 0;
    }
    MCTS(bool url) {
        srand(time(NULL));
        time_limit = 1000;
        opponent = -1;
        use_rollout_limit = url;
        use_time_limit = !url;
        last_winloss = 5;
        node_count = 0;
    }

    void choose_rollout_limit() {
        use_rollout_limit = true;
        use_time_limit = false;
    }

    void choose_time_limit() {
        use_rollout_limit = false;
        use_time_limit = true;
    }

    // SETTERS
    void set_time_limit(long long tl) {
        time_limit = tl;
    }

    void set_opponent(const int_fast8_t i) {
        opponent = i;
    }

    void set_readout(bool b) {
        readout = b;
    }

    void set_rollout_limit(long long rl) {
        rollout_limit = rl;
    }

    void set_nodes(int_fast32_t n) {
        node_count = n;
    }

    auto get_nodes() -> int_fast32_t {
        return node_count;
    }

    // GETTERS
    double get_most_recent_winrate() {
        return last_winloss;
    }

    void deleteTree(Node<StateType>* root) {
        /* first delete the subtrees */
        for (Node<StateType>* child : root->children) {
            deleteTree(child);
        }
        /* then delete the node */
        delete root;
    }

    auto prune(Node<StateType>* parent, const StateType& target) -> Node<StateType>* {
        Node<StateType>* out = nullptr;
        bool found = false;
        for (Node<StateType>* child : parent->get_children()) {
            if (!found && child->get_state() == target) {
                out = child;
                found = true;
            } else
                deleteTree(child);
        }
        if (out)
            out->set_parent(nullptr);
        delete parent;
        return out;
    }

    auto find_best_next_board(const StateType board) -> StateType {
        // board is immediately copied ^^^

        node_count = 0;
        set_opponent(-board.get_turn());

        // tracks time
        auto start = std::chrono::steady_clock::now();
        auto end = start + std::chrono::milliseconds(time_limit);

        Node<StateType>* rootNode = new Node<StateType>(board);
        rootNode->set_state(board);
        rootNode->set_player_no(opponent);

        do {
            select_expand_simulate_backpropagate(rootNode);
            node_count++;
        } while (
            (!use_time_limit || std::chrono::steady_clock::now() < end) 
            && (!use_rollout_limit || node_count < rollout_limit));

        StateType out = rootNode->best_child()->get_state();
        
        if (readout) {
            auto time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();
            std::cout << node_count << " nodes processed in " << time << "ms at " << (double)node_count / ((double)time / 1000.0) << "NPS.\n";
        }

        deleteTree(rootNode);
        return out;
    }

    auto getRolloutCounts(const StateType board) -> std::vector<int> {
        // board is immediately copied ^^^

        node_count = 0;
        set_opponent(-board.get_turn());

        // tracks time
        auto start = std::chrono::steady_clock::now(); 
        auto end = start + std::chrono::milliseconds(time_limit);

        auto rootNode = new Node<StateType>(board);
        rootNode->set_state(board);
        rootNode->set_player_no(opponent);

        do {
            select_expand_simulate_backpropagate(rootNode);
            node_count++;
        } while ((!use_time_limit || std::chrono::steady_clock::now() < end) && (!use_rollout_limit || node_count < rollout_limit));

        last_winloss = rootNode->best_child()->get_winrate();
        last_winloss = std::max(last_winloss, 0.0);

        std::vector<int> out(rootNode->get_children().size());
        for (int child_idx = 0; child_idx < rootNode->get_children().size(); child_idx++) {
            out[child_idx] = rootNode->get_children()[child_idx].get_visit_count();
        }

        return out;
    }

    inline void select_expand_simulate_backpropagate(Node<StateType>* rootNode) {
        int result;
        // SELECTION
        Node<StateType>* promisingNode = select_promising_node(rootNode);

        // EXPANSION
        if (!promisingNode->board.is_game_over())
            promisingNode->expand();

        Node<StateType>* nodeToExplore = promisingNode;

        if (promisingNode->get_children().size() != 0)
            nodeToExplore = promisingNode->random_child();

        // SIMULATION
        result = simulate_playout(nodeToExplore);
        // BACKPROPAGATION
        backprop(nodeToExplore, result);
    }

    inline auto select_promising_node(Node<StateType>* const rootNode) -> Node<StateType>* {
        Node<StateType>* node = rootNode;
        while (node->get_children().size())
            node = UCT<Node<StateType>, UCT_EXP_FACTOR>::best_node_uct(node);
        return node;
    }

    inline void backprop(Node<StateType>* nodeToExplore, const int_fast8_t winner) {
        Node<StateType>* propagator = nodeToExplore;
        while (propagator) {
            propagator->increment_visits();
            if (propagator->get_player_no() == winner) {
                propagator->add_score(REWARD);
            }
            propagator = propagator->get_parent();
        }
    }

    inline auto simulate_playout(Node<StateType>* node) -> int_fast8_t {
        StateType playout_board = node->get_state();
        playout_board.mem_setup();
        uint_fast8_t status = playout_board.evaluate();
        if (status == opponent) {
            // tests for an immediate loss in the position and sets to MIN_VALUE
            // if there is one.
            node->get_parent()->set_win_score(INT_MIN);
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

template <class StateType, int UCT_EXP_FACTOR = 6>
class Zero {
    SearchDriver::MCTS<StateType, UCT_EXP_FACTOR> search_driver = SearchDriver::MCTS<StateType, UCT_EXP_FACTOR>();
    StateType node = StateType();

public:
    Zero() {
        Zero(99);
    }
    Zero(const long long str) {
        search_driver.set_time_limit(str);
    }

    void set_time_limit(long long x) {
        search_driver.set_time_limit(x);
    }
    void choose_time_limit() {
        search_driver.choose_time_limit();
    }
    void set_rollout_limit(long long x) {
        search_driver.set_rollout_limit(x);
    }
    void choose_rollout_limit() {
        search_driver.choose_rollout_limit();
    }
    void set_readout(bool b) {
        search_driver.set_readout(b);
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

    void set_node(StateType n) {
        node = n;
    }

    void reset_node() {
        node.reset();
    }

    void show_node() {
        node.show();
    }

    auto node_eval() {
        return node.evaluate();
    }

    auto is_game_over() {
        return node.is_game_over();
    }

    auto get_node_count() {
        return search_driver.get_nodes();
    }

    void engine_move() {
        node = search_driver.find_best_next_board(node);
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