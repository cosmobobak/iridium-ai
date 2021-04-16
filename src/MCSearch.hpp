#pragma once

#include <chrono>

#include "TreeNode.hpp"
#include "UCT.hpp"

using namespace TreeNode;

namespace SearchDriver {
constexpr uint_fast8_t WIN_SCORE = 10;

template <class StateType, int UCT_EXP_FACTOR = 6>
class MCTS {
    long long time_limit;        // limiter on search time
    long long rollout_limit;     // limiter on rollouts
    const bool memsafe = true;  // dictates whether we preserve a part of the tree across moves
    uint_fast8_t opponent;      // the win score that the opponent wants
    int_fast32_t node_count = 0;
    Node<StateType>* preservedNode = nullptr;
    bool use_rollout_limit;
    bool use_time_limit;

   public:
    MCTS() {
        MCTS(1);
    }
    MCTS(const int_fast8_t player) {
        MCTS(player, 3);
    }
    MCTS(const int_fast8_t player, const long long strength) {
        srand(time(NULL));
        time_limit = strength;
        opponent = -player;
        use_rollout_limit = false;
        use_time_limit = true;
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
    }

    void set_time_limit(long long tl) {
        time_limit = tl;
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

    void set_opponent(const int_fast8_t i) {
        opponent = i;
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
        node_count = 0;
        set_opponent(-board.get_turn());
        // an end time which will act as a terminating condition
        auto end = std::chrono::steady_clock::now() + std::chrono::milliseconds(time_limit);
        Node<StateType>* rootNode = nullptr;
        if (preservedNode) {
            rootNode = prune(preservedNode, board);
        }
        if (!rootNode) {
            rootNode = new Node<StateType>(board);
            rootNode->set_state(board);
            rootNode->set_player_no(opponent);
        }
        // if you start getting weird out_of_range() errors at low TC then expand the root node here
        // auto breakunit = std::chrono::steady_clock::now();
        while ((!use_time_limit || std::chrono::steady_clock::now() < end) && (!use_rollout_limit || node_count < rollout_limit)) {
            Node<StateType>* promisingNode = select_promising_node(rootNode);

            if (!promisingNode->board.is_game_over())
                expand_node(promisingNode);

            Node<StateType>* nodeToExplore = promisingNode;

            if (promisingNode->get_children().size() != 0)
                nodeToExplore = promisingNode->random_child();

            uint_fast8_t result = simulate_playout(nodeToExplore);
            backprop(nodeToExplore, result);

            // if (std::chrono::steady_clock::now() > breakunit) {
            //     std::cout << "The best move so far is: " << rootNode->best_child_as_move() + 1 << '\n';
            //     breakunit += std::chrono::milliseconds(500);
            //     rootNode->show_child_visitrates();
            // }
            node_count++;
        }
        StateType out = rootNode->best_child()->get_state();
        // std::cout << "ZERO:\n";
        std::cout << node_count << " nodes processed at " << (double)node_count / ((double)time_limit / 1000.0) << "NPS.\n";
        // std::cout << nodes << ", ";
        // std::cout << "Zero win prediction: " << (int)(rootNode->best_child()->get_winrate() * (100 / WIN_SCORE)) << "%\n";
        uint_fast8_t action, sboard, square, row, col;
        action = rootNode->best_child_as_move();
        // assert(action >= 0 && action <= 80);
        // std::cout << action << '\n';
        square = action % 9;
        sboard = action / 9;
        col = (sboard % 3) * 3 + square % 3;
        row = (sboard / 3) * 3 + square / 3;
        std::cout << row << " " << col << std::endl;
        // rootNode->show_child_visitrates();
        if (!memsafe) {
            deleteTree(rootNode);
        } else {
            preservedNode = prune(rootNode, out);
        }
        return out;
    }

    inline auto select_promising_node(Node<StateType>* const rootNode) -> Node<StateType>* {
        Node<StateType>* node = rootNode;
        while (node->get_children().size() != 0)
            node = UCT<Node<StateType>, UCT_EXP_FACTOR>::best_node_uct(node);
        return node;
    }

    inline void expand_node(Node<StateType>* node) {
        node->expand();
    }

    inline void backprop(Node<StateType>* nodeToExplore, const int_fast8_t winner) {
        Node<StateType>* propagator = nodeToExplore;
        while (propagator) {
            propagator->increment_visits();
            if (propagator->get_player_no() == winner) {
                propagator->add_score(WIN_SCORE);
            }
            propagator = propagator->get_parent();
        }
    }

    inline auto simulate_playout(Node<StateType>* node) -> int_fast8_t {
        StateType tempState = node->get_state();
        tempState.mem_setup();
        uint_fast8_t status = tempState.evaluate();
        if (status == opponent) {
            node->get_parent()->set_win_score(INT_MIN);
            return status;
        }
        while (!tempState.is_game_over()) {
            tempState.random_play();
        }
        status = tempState.evaluate();
        return status;
    }
};
};  // namespace SearchDriver

// Possible heuristic improvement: use a long search to generate MCTS values for each starting square, use them as a heuristic starter.
// The RAVE approach makes this heuristic value = some sort of aggregate score of the move on parent nodes.
// UCT becomes (simulation value / rollouts) + (heuristic value / rollouts) + (exploration factor)

namespace MCSearch {

template <class StateType, int UCT_EXP_FACTOR = 6>
class Zero {
   public:
    auto searchDriver = SearchDriver::MCTS<StateType, UCT_EXP_FACTOR>();
    StateType node = StateType();

    Zero() {
        Zero(99);
    }
    Zero(const long long strength) {
        searchDriver.set_time_limit(strength);
    }

    inline void print(const std::string input, const std::string end = "\n") {
        std::cout << input << end;
    }

    auto get_player_move() {
        return node.get_player_move();
    }

    void engine_move() {
        node = searchDriver.find_best_next_board(node);
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