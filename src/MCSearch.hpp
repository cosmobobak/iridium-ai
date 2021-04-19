#pragma once

#include <chrono>

#include "TreeNode.hpp"
#include "UCT.hpp"

using namespace TreeNode;

namespace SearchDriver {
constexpr int REWARD = 10;

template <class StateType, int UCT_EXP_FACTOR>
class MCTS {
    // limiter on search time
    long long time_limit;
    // limiter on rollouts
    long long rollout_limit;

    // the win score that the opponent wants
    int opponent;

    // flags
    bool readout = true;
    bool debug = true;
    bool use_rollout_limit;
    bool use_time_limit;

    // recorded search data
    // the win / loss ratio of the most recently played move
    double last_winloss;
    int node_count;

    // dictates whether we preserve a part of the tree across moves
    bool memsafe = true;
    Node<StateType>* preservedNode = nullptr;

   public:
    MCTS() {
        MCTS(1, 3);
    }
    MCTS(int player, long long strength) {
        srand(time(NULL));
        time_limit = strength;
        opponent = -player;
        use_rollout_limit = false;
        use_time_limit = true;
        last_winloss = 5;
        node_count = 0;
    }
    MCTS(int player, long long strength, bool use_rollout_limit) {
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

    void set_opponent(int i) {
        opponent = i;
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
        for (Node<StateType>* child : parent->children) {
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
        // board is immediately copied        ^^^

        node_count = 0;
        opponent = -board.get_turn();

        // tracks time
        auto start = std::chrono::steady_clock::now();
        auto end = start + std::chrono::milliseconds(time_limit);

        Node<StateType>* root_node = new Node<StateType>(board);
        root_node->set_state(board);
        root_node->set_player_no(opponent);

        do {
            select_expand_simulate_backpropagate(root_node);
            node_count++;
            if (debug && (node_count & 0b111111111111111) == 0b111111111111111) {
                root_node->show_child_visitrates();
                std::cout << "| ";
                root_node->show_child_winrates();
                std::cout << "| ";
                for (auto child : root_node->get_children()) {
                    std::cout << UCT<Node<StateType>, UCT_EXP_FACTOR>::compute_uct(child) << " ";
                }
                std::cout << "\n";
            }

        } while (
            (!use_time_limit || std::chrono::steady_clock::now() < end) 
            && (!use_rollout_limit || node_count < rollout_limit));

        StateType out = root_node->best_child()->get_state();
        
        if (readout) {
            auto time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();
            std::cout << node_count << " nodes processed in " << time << "ms at " << (double)node_count / ((double)time / 1000.0) << "NPS.\n";
        }

        deleteTree(root_node);
        return out;
    }

    auto get_rollout_counts(const StateType board) -> std::vector<int> {
        // board is immediately copied      ^^^

        node_count = 0;
        opponent = -board.get_turn();

        // tracks time
        auto start = std::chrono::steady_clock::now(); 
        auto end = start + std::chrono::milliseconds(time_limit);

        Node<StateType>* root_node = new Node<StateType>(board);
        root_node->set_state(board);
        root_node->set_player_no(opponent);

        do {
            select_expand_simulate_backpropagate(root_node);
            node_count++;
        } while ((!use_time_limit || std::chrono::steady_clock::now() < end) && (!use_rollout_limit || node_count < rollout_limit));

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

    inline void select_expand_simulate_backpropagate(Node<StateType>* root_node) {
        int result;
        // SELECTION
        Node<StateType>* promisingNode = select_promising_node(root_node);

        // EXPANSION
        if (!promisingNode->board.is_game_over())
            promisingNode->expand();

        Node<StateType>* nodeToExplore = promisingNode;

        if (promisingNode->children.size() != 0)
            nodeToExplore = promisingNode->random_child();

        // SIMULATION
        result = simulate_playout(nodeToExplore);
        // BACKPROPAGATION
        backprop(nodeToExplore, result);
    }

    inline auto select_promising_node(Node<StateType>* root_node) const -> Node<StateType>* {
        Node<StateType>* node = root_node;
        while (node->children.size())
            node = UCT<Node<StateType>, UCT_EXP_FACTOR>::best_node_uct(node);
        return node;
    }

    inline void backprop(Node<StateType>* nodeToExplore, int winner) {
        Node<StateType>* propagator = nodeToExplore;
        while (propagator) {
            propagator->increment_visits();
            if (propagator->get_player_no() == winner) {
                propagator->add_score(REWARD);
            }
            propagator = propagator->get_parent();
        }
    }

    inline auto simulate_playout(Node<StateType>* node) -> int {
        StateType playout_board = node->get_state();
        playout_board.mem_setup();
        int status = playout_board.evaluate();
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
    void set_debug(bool b) {
        search_driver.set_debug(b);
    }

    inline void print(const std::string input, const std::string end = "\n") {
        std::cout << input << end;
    }

    auto get_player_move() {
        return node.get_player_move();
    }

    auto get_node() -> StateType* {
        return &node;
    }

    auto turn_modifier() {
        return node.get_turn();
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

    auto rollout_vector(StateType node) {
        std::vector<int> child_rollout_counts = search_driver.get_rollout_counts(node);
        std::vector<int> out(7);
        int idx = 0;
        for (int move : node.legal_moves()) {
            out[move] = child_rollout_counts[idx++];
        }
        return out;
    }

    auto get_win_prediction() -> double {  
        // multiplies by 10 to get a weighted win-per-node percentage
        return 10 * search_driver.get_most_recent_winrate();
    }

    auto make_sample_move(std::vector<int> dist, StateType model) {
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