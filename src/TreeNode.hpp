#pragma once

#include <cassert>
#include <chrono>
#include <random>

#include "games/Connect4-4x4.hpp"
#include "games/Connect4.hpp"
// #include "games/Gomoku.hpp"
#include "games/TicTacToe.hpp"
#include "games/UTTT2.hpp"
#include "utilities/rng.hpp"

namespace TreeNode {

template <class State>
    class TreeNode {
        using Move = typename State::Move;
        State board;
        std::vector<TreeNode*> children;
        TreeNode* parent = nullptr;
        int win_count = 0;
        int visits = 0;
        int turn;

       public:
        TreeNode(const State& board) {
            this->board = board;
            set_player_no(-board.get_turn());
        }
        TreeNode(const TreeNode&) = delete;
        TreeNode(TreeNode&&) = delete;
        ~TreeNode() noexcept {
            for (auto child : children) {
                delete child;
            }
        }

        TreeNode(const State& board, TreeNode* parent, int turn) : board(board), parent(parent), turn(turn) {}

        // SETTERS
        void set_state(const State& board) {
            this->board = board;
        }

        void set_parent(TreeNode* parent) {
            this->parent = parent;
        }

        void set_player_no(int turn) {
            this->turn = turn;
        }

        void set_win_score(int win_count) {
            this->win_count = win_count;
        }

        // GETTERS
        auto get_state() -> State& {
            return board;
        }

        auto copy_state() const -> State {
            return board;
        }

        auto get_children() const -> const std::vector<TreeNode*>& {
            return children;
        }

        auto get_parent() const -> TreeNode* {
            return parent;
        }

        auto get_player_no() const -> int {
            return turn;
        }

        auto get_opponent() const -> int {
            return -turn;
        }

        auto get_win_score() const -> int {
            return win_count;
        }

        auto get_visit_count() const -> int {
            return visits;
        }

        auto get_winrate() const -> double {
            return (double)win_count / (double)visits;
        }

        auto get_parent_visits() const -> int {
            return parent->get_visit_count();
        }

        // INTERACTIONS
        void add_score(int s) {
            win_count += s;
        }

        void increment_visits() {
            ++visits;
        }

        auto random_child() const -> TreeNode* {
            assert(!children.empty());
            return rng::choice(children);
        }

        void expand() {
            assert(board.num_legal_moves() == board.legal_moves().size());
            children.reserve(board.num_legal_moves());
            for (int move : board.legal_moves()) {
                board.play(move);
                children.push_back(new TreeNode(board, this, get_opponent()));
                board.unplay(move);
            }
        }

        auto best_child() const -> TreeNode* {
            auto max = std::max_element(
                children.begin(), children.end(),
                [](const TreeNode* a, const TreeNode* b) { return (a->get_visit_count() < b->get_visit_count()); });
            return *max;
        }

        auto best_child_as_move() const -> Move {
            auto result = std::max_element(
                children.begin(), children.end(),
                [](const TreeNode* a, const TreeNode* b) { return (a->get_visit_count() < b->get_visit_count()); });
            return board.legal_moves()[std::distance(children.begin(), result)];
        }

        // DEBUG
        void show() {
            std::cout << "My state is:\n";
            board.show();
            if (parent) {
                std::cout << "My parent's state is:\n";
                parent->show();
            }
            std::cout << "and I have " << children.size() << " children.\n";
        }

        void show_child_winrates() const {
            for (const auto& child : children) {
                std::cout << (child->get_win_score() * 10) / child->get_visit_count() << " ";
            }
        }

        void show_child_visitrates() const {
            for (const auto& child : children) {
                std::cout << child->get_visit_count() << " ";
            }
        }

        void print_tree(int depth = 0) const {
            if (visits == 0 || depth > 2) {
                return;
            }
            for (int i = 0; i < depth; ++i) {
                if (i == depth - 1) {
                    printf("├─");
                } else {
                    printf("  ");
                }
            }
            printf("visits: %d ", get_visit_count());
            printf("win_count: %d ", get_win_score());
            printf("winrate: %.2f ", get_winrate());
            printf("player: %d ", get_player_no());
            printf("\n");
            for (const auto& child : children) {
                child->print_tree(depth + 1);
            }
        }

        void print_pv() {
            // print the principal variation from this node
            auto pv = std::vector<Move>();
            auto node = this;
            for (; !node->children.empty(); node = node->best_child()) {
                pv.push_back(node->best_child_as_move());
            }
            printf("PV: ");
            for (auto move : pv) {
                printf("%d ", move + 1);
            }
            if (node->get_state().is_game_over()) {
                int win1 = node->get_state().evaluate() == 1;
                int win2 = node->get_state().evaluate() == -1;
                if (win1 || win2) {
                    printf("(%d-%d) ", win1, win2);
                } else {
                    printf("(1/2-1/2) ");
                }
            }
            printf("\n");
        }
};
}