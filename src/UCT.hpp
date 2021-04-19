#pragma once

#include <climits>
#include <cmath>

#include "Connect4-4x4.hpp"
#include "Connect4.hpp"
#include "Gomoku.hpp"
#include "RawTree.hpp"
#include "TicTacToe.hpp"
#include "TreeNode.hpp"
#include "UTTT.hpp"

using namespace TreeNode;

template <class NodeType, int EXP_FACTOR>
class UCT {
   public:
    inline static auto uct_value(int parent_visits, double win_count, int visits) -> double {
        if (visits == 0) {
            return INT_MAX;
        }
        return (win_count / (double)visits) + sqrt(log(parent_visits) / (double)visits) * EXP_FACTOR;
    }

    inline static auto compute_uct(const NodeType* a) -> double {
        return uct_value(
            a->get_parent_visits(),
            a->get_win_score(),
            a->get_visit_count());
    }

    inline static auto compare_ucb1(const NodeType* a, const NodeType* b) -> bool {
        return uct_value(
                   a->get_parent_visits(),
                   a->get_win_score(),
                   a->get_visit_count()) <
               uct_value(
                   b->get_parent_visits(),
                   b->get_win_score(),
                   b->get_visit_count());
    }
    
    inline static auto best_node_uct(const NodeType* node) -> NodeType* {
        return *std::max_element(
            node->children.begin(),
            node->children.end(),
            compare_ucb1);
    }
};