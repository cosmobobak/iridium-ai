#pragma once

#include "Connect4-4x4.hpp"
#include "Connect4.hpp"
#include "Gomoku.hpp"
#include "RawTree.hpp"
#include "TicTacToe.hpp"
#include "UTTT.hpp"

namespace TreeNode {
template <class State>
class Node {
   public:
    State board;
    std::vector<Node*> children;
    Node* parent = nullptr;
    int_fast32_t win_count = 0;
    uint_fast32_t visits = 0;
    int_fast8_t turn;

    Node(const State& board) {
        this->board = board;
    }
    ~Node() {
        for (Node* child : children) {
            delete child;
        }
    }

    // SETTERS
    inline void set_state(const State& board) {
        this->board = board;
    }

    inline void set_parent(Node* parent) {
        this->parent = parent;
    }


    inline void set_player_no(const int turn) {
        this->turn = turn;
    }

    inline void set_win_score(const int_fast32_t s) {
        win_count = s;
    }

    // GETTERS
    inline auto get_state() const -> State {
        return board;
    }

    inline auto get_children() const -> std::vector<Node*> {
        return children;
    }

    inline auto get_parent() const -> Node* {
        return parent;
    }

    inline auto get_player_no() const -> int {
        return turn;
    }

    inline auto get_opponent() const -> int {
        return -turn;
    }

    inline auto get_win_score() const -> int_fast32_t {
        return win_count;
    }

    inline auto get_visit_count() const -> int_fast32_t {
        return visits;
    }

    inline auto get_winrate() const -> double {
        return (double)win_count / (double)visits;
    }

    inline auto get_parent_visits() const -> int_fast32_t {
        return parent->get_visit_count();
    }

    // INTERACTIONS
    inline void add_score(const int_fast32_t s) {
        win_count += s;
    }

    inline void increment_visits() {
        visits++;
    }

    inline auto random_child() -> Node* {
        return children[rand() % children.size()];
    }

    inline void expand() {
        children.resize(board.num_legal_moves());
        int x = 0;
        for (int move : board.legal_moves()) {
            board.play(move);
            children[x] = new Node(board);
            children[x]->set_parent(this);
            children[x]->set_player_no(get_opponent());
            board.unplay();
            x++;
        }
    }

    inline auto best_child() -> Node* {
        typename std::vector<Node*>::iterator result;
        result = std::max_element(
            children.begin(), children.end(),
            [](Node* a, Node* b) { return (a->get_visit_count() < b->get_visit_count()); });
        return *result;
    }

    inline auto best_child_as_move() {
        typename std::vector<Node*>::iterator result;
        result = std::max_element(
            children.begin(), children.end(),
            [](Node* a, Node* b) { return (a->get_visit_count() < b->get_visit_count()); });
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

    inline void show_child_winrates() const {
        for (const auto& child : children) {
            std::cout << (child->get_win_score() * 10) / child->get_visit_count() << " ";
        }
    }

    inline void show_child_visitrates() const {
        for (const auto& child : children) {
            std::cout << child->get_visit_count() << " ";
        }
    }
};
}