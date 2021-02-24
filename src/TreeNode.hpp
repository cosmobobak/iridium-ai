#pragma once

#include "Connect4-4x4.hpp"
#include "Connect4.hpp"
#include "Gomoku.hpp"
#include "RawTree.hpp"
#include "TicTacToe.hpp"
#include "UTTT.hpp"

namespace TreeNode {
template <class ContainedState>
class Node {
   public:
    int_fast32_t winScore = 0;
    int_fast32_t visits = 0;
    uint_fast8_t playerNo;
    ContainedState board;
    Node* parent = nullptr;
    std::vector<Node*> children;

    Node(const ContainedState& board) {
        this->board = board;
    }

    inline void set_player_no(const uint_fast8_t playerNo) {
        this->playerNo = playerNo;
    }

    inline auto get_player_no() const -> uint_fast8_t {
        return playerNo;
    }

    inline auto get_opponent() const -> uint_fast8_t {
        return -playerNo;
    }

    inline void set_parent(Node* parent) {
        this->parent = parent;
    }

    inline void set_state(const ContainedState& board) {
        this->board = board;
    }

    void show() {
        std::cout << "My state is:\n";
        board.show();
        if (parent) {
            std::cout << "My parent's state is:\n";
            parent->show();
        }
        std::cout << "and I have " << children.size() << " children.\n";
    }

    inline auto get_children() const -> std::vector<Node*> {
        return children;
    }

    inline void expand() {
        children.reserve(board.num_legal_moves());
        for (const auto& move : board.legal_moves()) {
            board.play(move);
            children.push_back(new Node(board));
            children.back()->set_parent(this);
            children.back()->set_player_no(get_opponent());
            board.unplay();
        }
    }

    inline auto get_win_score() const -> int_fast32_t {
        return winScore;
    }

    inline auto get_visit_count() const -> int_fast32_t {
        return visits;
    }

    inline auto get_parent_visits() const -> int_fast32_t {
        return parent->get_visit_count();
    }

    inline void increment_visits() {
        visits++;
    }

    inline void add_score(const int_fast32_t s) {
        winScore += s;
    }

    inline void set_win_score(const int_fast32_t s) {
        winScore = s;
    }

    inline auto get_parent() const -> Node* {
        return parent;
    }

    inline auto get_state() const -> ContainedState {
        return board;
    }

    inline auto random_child() -> Node* {
        return children[rand() % children.size()];
    }

    inline auto get_winrate() const -> double {
        return (double)winScore / (double)visits;
    }

    inline auto best_child() -> Node* {
        typename std::vector<Node*>::iterator result;
        result = std::max_element(
            children.begin(), children.end(),
            [](Node* a, Node* b) { return (a->get_visit_count() < b->get_visit_count()); });
        return children[std::distance(children.begin(), result)];
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

    inline void show_child_winrates() const {
        for (const auto& child : children) {
            std::cout << child->get_win_score() << " ";
        }
        std::cout << "\n";
    }

    inline void show_child_visitrates() const {
        for (const auto& child : children) {
            std::cout << child->get_visit_count() << " ";
        }
        std::cout << "\n";
    }
};
}