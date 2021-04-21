#pragma once

#include <limits>
#include <cmath>

#include "Connect4-4x4.hpp"
#include "Connect4.hpp"
#include "Gomoku.hpp"
#include "RawTree.hpp"
#include "TicTacToe.hpp"
#include "TreeNode.hpp"
#include "UTTT.hpp"

template <class Node, int EXP_FACTOR>
class UCT {
   public:
    static auto uct_value(int parent_visits, double win_count, int visits) -> double {
        if (visits == 0) {
            return std::numeric_limits<double>::max();
        }
        return (win_count / (double)visits) + sqrt(log(parent_visits) / (double)visits) * EXP_FACTOR;
    }

    static auto compute_uct(const Node* a) -> double {
        return uct_value(
            a->get_parent_visits(),
            a->get_win_score(),
            a->get_visit_count());
    }

    static auto compare_ucb1(const Node* a, const Node* b) -> bool {
        return uct_value(
                   a->get_parent_visits(),
                   a->get_win_score(),
                   a->get_visit_count()) <
               uct_value(
                   b->get_parent_visits(),
                   b->get_win_score(),
                   b->get_visit_count());
    }

    static auto best_node_uct(const Node* node) -> Node* {
        return *std::max_element(
            node->children.begin(),
            node->children.end(),
            compare_ucb1);
    }
};