#pragma once

#include <cmath>
#include <limits>

#include "Connect4-4x4.hpp"
#include "Connect4.hpp"
#include "Gomoku.hpp"
#include "RawTree.hpp"
#include "TicTacToe.hpp"
#include "TreeNode.hpp"
#include "UTTT.hpp"

template <class Node, int EXP_FACTOR, int NUM_MOVE_OPTIONS>
class RAVE {
    static std::array<int, NUM_MOVE_OPTIONS> playout_wins;
    static std::array<int, NUM_MOVE_OPTIONS> playout_counts;

   public:
    static auto rave_value(int parent_visits, int win_count, int visits) -> double {
        if (visits == 0) {
            return std::numeric_limits<double>::max();
        }
        double exploitation = (double)win_count / (double)visits;
        double exploration = sqrt(log((double)parent_visits) / (double)visits) * EXP_FACTOR;
        // rapid action value estimation
        double rave = 0;
        return exploitation + exploration + rave;
    }

    static auto compute_rave(const Node* a) -> double {
        return rave_value(
            a->get_parent_visits(),
            a->get_win_score(),
            a->get_visit_count());
    }

    static auto compare_rave(const Node* a, const Node* b) -> bool {
        return rave_value(
                   a->get_parent_visits(),
                   a->get_win_score(),
                   a->get_visit_count()) <
               rave_value(
                   b->get_parent_visits(),
                   b->get_win_score(),
                   b->get_visit_count());
    }

    static auto best_child_rave(const Node* node) -> Node* {
        return *std::max_element(
            node->get_children().begin(),
            node->get_children().end(),
            compare_rave);
    }
};