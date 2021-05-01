#pragma once

#include "Connect4-4x4.hpp"
#include "Connect4.hpp"
#include "Gomoku.hpp"
#include "RawTree.hpp"
#include "TicTacToe.hpp"
#include "UTTT.hpp"

namespace TreeNode {
template <class State>
class TreeNode {
    State board;
    std::vector<TreeNode*> children;
    TreeNode* parent = nullptr;
    int win_count = 0;
    int visits = 0;
    int turn;

public:
    TreeNode(const State& board) {
        this->board = board;
    }
    TreeNode(const TreeNode&) = delete;
    ~TreeNode() noexcept {
        for (TreeNode* child : children) {
            delete child;
        }
    }

    // SETTERS
    void set_state(const State& board) {
        this->board = board;
    }

    void set_parent(TreeNode* parent) {
        this->parent = parent;
    }


    void set_player_no(const int turn) {
        this->turn = turn;
    }

    void set_win_score(const int s) {
        win_count = s;
    }

    // GETTERS
    auto get_state() const -> const State& {
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
        visits++;
    }

    auto random_child() -> TreeNode* {
        assert(children.size() != 0);
        return children[rand() % children.size()];
    }

    void expand() {
        children.resize(board.num_legal_moves());
        int idx = 0;
        for (int move : board.legal_moves()) {
            board.play(move);
            children[idx] = new TreeNode(board);
            children[idx]->set_parent(this);
            children[idx]->set_player_no(get_opponent());
            board.unplay(move);
            idx++;
        }
    }

    auto best_child() -> TreeNode* {
        return *std::max_element(
            children.begin(), children.end(),
            [](TreeNode* a, TreeNode* b) { return (a->get_visit_count() < b->get_visit_count()); });
    }

    auto best_child_as_move() {
        typename std::vector<TreeNode*>::iterator result;
        result = std::max_element(
            children.begin(), children.end(),
            [](TreeNode* a, TreeNode* b) { return (a->get_visit_count() < b->get_visit_count()); });
        // if (board.legal_moves().size() != children.size()) {
        //     std::cout << board.legal_moves().size() << " " << children.size() << '\n';
        //     board.show_debug();
        //     std::terminate();
        // }
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
};
}