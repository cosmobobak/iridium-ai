#pragma once

#include <cmath>
#include <limits>

#include "games/Connect4-4x4.hpp"
#include "games/Connect4.hpp"
// #include "games/Gomoku.hpp"
#include "games/TicTacToe.hpp"
#include "games/UTTT2.hpp"

template <class Node, int EXP_FACTOR>
class UCT {
   public:
    static auto ucb1_value(int parent_visits, int win_count, int visits) -> double {
        if (visits == 0) {
            return std::numeric_limits<double>::max() - 1;
        }
        double exploitation = (double)win_count / (double)visits;
        double exploration = sqrt(log((double)parent_visits) / (double)visits) * EXP_FACTOR;
        return exploitation + exploration;
    }

    static auto compute_ucb1(const Node* a) -> double {
        return ucb1_value(
            a->get_parent_visits(),
            a->get_win_score(),
            a->get_visit_count());
    }

    static auto compare_ucb1(const Node* a, const Node* b) -> bool {
        return ucb1_value(
                   a->get_parent_visits(),
                   a->get_win_score(),
                   a->get_visit_count()) <
               ucb1_value(
                   b->get_parent_visits(),
                   b->get_win_score(),
                   b->get_visit_count());
    }

    static auto best_child_ucb1(const Node* node) -> Node* {
        return *std::max_element(
            node->get_children().begin(),
            node->get_children().end(),
            compare_ucb1);
    }
};