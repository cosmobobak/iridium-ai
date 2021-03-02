#pragma once

#include "Connect4-4x4.hpp"
#include "Connect4.hpp"
#include "Gomoku.hpp"
#include "RawTree.hpp"
#include "TicTacToe.hpp"
#include "TreeNode.hpp"
#include "UTTT.hpp"

using namespace TreeNode;

template <class NodeType, int EXP_FACTOR = 6>
class UCT {
   public:
    inline static auto uct_value(
        int totalVisit,
        double nodeWinScore,
        int nodeVisit) -> double {
        if (nodeVisit == 0) {
            return INT_MAX;
        }
        return (nodeWinScore / (double)nodeVisit) + sqrt(log(totalVisit) / (double)nodeVisit) * EXP_FACTOR;
    }

    inline static auto compute_uct(const NodeType* a) -> double {
        return uct_value(
            a->get_parent_visits(),
            a->get_win_score(),
            a->get_visit_count());
    }

    inline static auto best_node_uct(const NodeType* node) -> NodeType* {
        int maxpos = 0;
        double maxval = compute_uct(node->children[0]), currentUCT;
        for (int i = 1; i < node->children.size(); i++) {
            currentUCT = compute_uct(node->children[i]);
            if (currentUCT > maxval) {
                maxpos = i;
                maxval = currentUCT;
            }
        }
        return node->children[maxpos];
    }
};