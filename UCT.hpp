#pragma once

#include "Connect4-4x4.hpp"
#include "Connect4.hpp"
#include "Gomoku.hpp"
#include "RawTree.hpp"
#include "TicTacToe.hpp"
#include "UTTT.hpp"

#include "TreeNode.hpp"

using namespace TreeNode;

constexpr auto EXP_FACTOR = gameexpfactor;

namespace UCT {
inline auto uct_value(
    const int totalVisit,
    const double nodeWinScore,
    const int nodeVisit) -> double {
    if (nodeVisit == 0) {
        return INT_MAX;
    }
    return (nodeWinScore / (double)nodeVisit) + 1.41 * sqrt(log(totalVisit) / (double)nodeVisit) * EXP_FACTOR;
}

inline auto compute_uct(const Node* a) -> double {
    return uct_value(
            a->get_parent_visits(),
            a->get_win_score(),
            a->get_visit_count());
}

inline auto best_node_uct(const Node* node) -> Node* {
    int maxpos = 0;
    double maxval = compute_uct(node->children[0]), currentUCT;
    for (int i = 1; i < node->children.size(); i++)
    {
        currentUCT = compute_uct(node->children[i]);
        if (currentUCT > maxval) {
            maxpos = i;
            maxval = currentUCT;
        }
    }
    return node->children[maxpos];
}
}  // namespace UCT