#pragma GCC optimize("Ofast", "unroll-loops", "inline")
#pragma GCC target("avx")

#include <algorithm>
#include <cassert>
#include <chrono>
#include <climits>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <optional>
#include <unordered_map>
#include <vector>

#include "Connect4-4x4.hpp"
#include "Connect4.hpp"
#include "Gomoku.hpp"
#include "RawTree.hpp"
#include "TicTacToe.hpp"
#include "UTTT.hpp"

// iridium-ai.cpp : This file contains the 'main' function. Program execution begins and ends there.

using namespace UTTT;

constexpr auto EXP_FACTOR = gameexpfactor;

class TreeNode {
   public:
    int_fast32_t winScore = 0;
    int_fast32_t visits = 0;
    int_fast8_t playerNo;
    State board;
    TreeNode* parent = nullptr;
    std::vector<TreeNode*> children;

    TreeNode(const State& board) {
        this->board = board;
    }

    void set_player_no(const int_fast8_t playerNo) {
        this->playerNo = playerNo;
    }

    auto get_player_no() const -> int_fast8_t {
        return playerNo;
    }

    auto get_opponent() const -> int_fast8_t {
        return -playerNo;
    }

    void set_parent(TreeNode* parent) {
        this->parent = parent;
    }

    void set_state(const State& board) {
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

    auto get_children() const -> std::vector<TreeNode*> {
        return children;
    }

    void expand() {
        children.reserve(board.num_legal_moves());
        for (const auto& move : board.legal_moves()) {
            board.play(move);
            children.push_back(new TreeNode(board));
            children.back()->set_parent(this);
            children.back()->set_player_no(get_opponent());
            board.unplay();
        }
    }

    auto get_win_score() const -> int_fast32_t {
        return winScore;
    }

    auto get_visit_count() const -> int_fast32_t {
        return visits;
    }

    auto get_parent_visits() const -> int_fast32_t {
        return parent->get_visit_count();
    }

    void increment_visits() {
        visits++;
    }

    void add_score(const int_fast32_t s) {
        winScore += s;
    }

    void set_win_score(const int_fast32_t s) {
        winScore = s;
    }

    auto get_parent() const -> TreeNode* {
        return parent;
    }

    auto get_state() const -> State {
        return board;
    }

    auto random_child() -> TreeNode* {
        return children[rand() % children.size()];
    }

    auto get_winrate() const -> double {
        return (double)winScore / (double)visits;
    }

    auto best_child() -> TreeNode* {
        std::vector<TreeNode*>::iterator result;
        result = std::max_element(
            children.begin(), children.end(),
            [](TreeNode* a, TreeNode* b) { return (a->get_visit_count() < b->get_visit_count()); });
        return children[std::distance(children.begin(), result)];
    }

    auto best_child_as_move() -> Move {
        std::vector<TreeNode*>::iterator result;
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

    void show_child_winrates() const {
        for (const auto& child : children) {
            std::cout << child->get_win_score() << " ";
        }
        std::cout << "\n";
    }

    void show_child_visitrates() const {
        for (const auto& child : children) {
            std::cout << child->get_visit_count() << " ";
        }
        std::cout << "\n";
    }
};

namespace UCT {
inline auto uct_value(
    const double totalVisit,
    const double nodeWinScore,
    const double nodeVisit) -> double {
    if (nodeVisit == 0) {
        return INT_MAX;
    }
    return (nodeWinScore / nodeVisit) + 1.41 * sqrt(log(totalVisit) / nodeVisit) * EXP_FACTOR;
}

inline auto uct_compare(const TreeNode* a, const TreeNode* b) -> bool {
    return (
        uct_value(
            a->get_parent_visits(),
            a->get_win_score(),
            a->get_visit_count()) <
        uct_value(
            b->get_parent_visits(),
            b->get_win_score(),
            b->get_visit_count()));
}

inline auto best_node_uct(const TreeNode* node) -> TreeNode* {
    auto children = node->get_children();
    std::vector<TreeNode*>::iterator result;
    result = std::max_element(
        children.begin(), children.end(),
        uct_compare);
    return children[std::distance(children.begin(), result)];
}
};  // namespace UCT
constexpr int_fast8_t WIN_SCORE = 10;
class MCTS {
   public:
    long long timeLimit;        // limiter on search time
    const bool memsafe = true;  // dictates whether we preserve a part of the tree across moves
    int_fast8_t opponent;       // the win score that the opponent wants
    int_fast32_t nodes = 0;
    TreeNode* preservedNode = nullptr;

    MCTS() {
        MCTS(1);
    }
    MCTS(const int_fast16_t player) {
        MCTS(player, 3);
    }
    MCTS(const int_fast16_t player, const long long strength) {
        srand(time(NULL));
        timeLimit = strength;
        opponent = -player;
    }

    void deleteTree(TreeNode* root) {
        /* first delete the subtrees */
        for (TreeNode* child : root->children) {
            deleteTree(child);
        }
        /* then delete the node */
        delete root;
    }

    void set_opponent(const int_fast16_t i) {
        opponent = i;
    }

    auto prune(TreeNode* parent, const State& target) -> TreeNode* {
        TreeNode* out = nullptr;
        bool found = false;
        for (TreeNode* child : parent->get_children()) {
            if (!found && child->get_state() == target) {
                out = child;
                found = true;
            } else
                deleteTree(child);
        }
        if (out)
            out->set_parent(nullptr);
        delete parent;
        return out;
    }

    auto find_best_next_board(const State board) -> State {
        nodes = 0;
        set_opponent(-board.get_turn());
        // an end time which will act as a terminating condition
        auto end = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeLimit);
        TreeNode* rootNode = nullptr;
        if (preservedNode)
            rootNode = prune(preservedNode, board);
        if (!rootNode) {
            rootNode = new TreeNode(board);
            rootNode->set_state(board);
            rootNode->set_player_no(opponent);
        }
        // if you start getting weird out_of_range() errors at low TC then expand the root node here
        // auto breakunit = std::chrono::steady_clock::now();
        while (std::chrono::steady_clock::now() < end) {
            TreeNode* promisingNode = select_promising_node(rootNode);

            if (!promisingNode->get_state().is_game_over())
                expand_node(promisingNode);

            TreeNode* nodeToExplore = promisingNode;

            if (promisingNode->get_children().size() != 0)
                nodeToExplore = promisingNode->random_child();

            int_fast8_t result = simulate_playout(nodeToExplore);
            backprop(nodeToExplore, result);

            // if (std::chrono::steady_clock::now() > breakunit) {
            //     std::cout << "The best move so far is: " << rootNode->best_child_as_move() + 1 << '\n';
            //     breakunit += std::chrono::milliseconds(500);
            //     rootNode->show_child_visitrates();
            // }
        }
        State out = rootNode->best_child()->get_state();
        // std::cout << "ZERO:\n";
        // std::cout << nodes << " nodes processed at " << (double)nodes / ((double)timeLimit / 1000.0) << "NPS.\n";
        // std::cout << nodes << ", ";
        // std::cout << "Zero win prediction: " << (int)(rootNode->best_child()->get_winrate() * (100 / WIN_SCORE)) << "%\n";
        int_fast8_t action, sboard, square, row, col;
        action = rootNode->best_child_as_move();
        // assert(action >= 0 && action <= 80);
        // std::cout << action << '\n';
        square = action % 9;
        sboard = action / 9;
        col = (sboard % 3) * 3 + square % 3;
        row = (sboard / 3) * 3 + square / 3;
        // std::cout << row << " " << col << std::endl;
        // rootNode->show_child_visitrates();
        if (!memsafe) {
            deleteTree(rootNode);
        } else {
            preservedNode = prune(rootNode, out);
        }
        return out;
    }

    inline auto select_promising_node(TreeNode* const rootNode) -> TreeNode* {
        TreeNode* node = rootNode;
        while (node->get_children().size() != 0)
            node = UCT::best_node_uct(node);
        return node;
    }

    inline void expand_node(TreeNode* node) {
        node->expand();
    }

    inline void backprop(TreeNode* nodeToExplore, const int_fast16_t winner) {
        TreeNode* propagator = nodeToExplore;
        while (propagator) {
            propagator->increment_visits();
            if (propagator->get_player_no() == winner) {
                propagator->add_score(WIN_SCORE);
            }
            propagator = propagator->get_parent();
        }
    }

    inline auto simulate_playout(TreeNode* node) -> int_fast16_t {
        nodes++;
        State tempState = node->get_state();
        tempState.mem_setup();
        int_fast8_t status = tempState.evaluate();
        if (status == opponent) {
            node->get_parent()->set_win_score(INT_MIN);
            return status;
        }
        while (!tempState.is_game_over()) {
            tempState.random_play();
        }
        status = tempState.evaluate();
        return status;
    }
};

class Zero {
   public:
    MCTS searchDriver = MCTS();
    State node = State();

    Zero() {
        Zero(99);
    }
    Zero(const long long strength) {
        searchDriver.timeLimit = strength;
    }

    inline void print(const std::string input, const std::string end = "\n") {
        std::cout << input << end;
    }

    auto get_player_move() -> Move {
        return node.get_player_move();
    }

    void engine_move() {
        node = searchDriver.find_best_next_board(node);
    }

    void show_result() const {
        // assert(node.evaluate() == node.evaluateOLD());
        switch (node.evaluate()) {
            case 0:
                std::cout << "1/2-1/2" << '\n';
                break;
            case 1:
                std::cout << "1-0" << '\n';
                break;
            case -1:
                std::cout << "0-1" << '\n';
                break;
            default:
                std::cerr << "evaluate returned non-zero";
                break;
        }
    }
};

// Possible heuristic improvement: use a long search to generate MCTS values for each starting square, use them as a heuristic starter.
// The RAVE approach makes this heuristic value = some sort of aggregate score of the move on parent nodes.
// UCT becomes (simulation value / rollouts) + (heuristic value / rollouts) + (exploration factor)

struct TTEntry {
    int_fast8_t depth;
    int_fast16_t score;
    int_fast8_t type;

    TTEntry(int_fast8_t d, int_fast16_t s, int_fast8_t t) {
        depth = d;
        score = s;
        type = t;
    }
};

class Istus {
   public:
    State node;
    int_fast32_t nodes;
    long long timeLimit;
    //std::unordered_map<State, TTEntry, State::HashFunction> hashtable;

    Istus() {
        Istus(99);
    }
    Istus(const long long TL) {
        timeLimit = TL;
    }

    // void record_hash(State key, int_fast8_t depth, int_fast16_t a, int hashDataType) {
    //     if (hashtable.contains(key)) {
    //         TTEntry entry = hashtable[key];
    //         if (entry.depth >= depth) {
    //             hashtable[key] = TTEntry(depth, a, hashDataType);
    //         }
    //     } else {
    //         hashtable[key] = TTEntry(depth, a, hashDataType);
    //     }
    // }

    // auto probe_hash(State key) -> std::optional<TTEntry> {
    //     if (hashtable.contains(key)) {
    //         return hashtable[key];
    //     } else {
    //         return std::nullopt;
    //     }
    // }

    // auto probe_hash_value(State key, int depth, int a, int b, bool &validityFlag) -> int {
    //     TTEntry entry = hashtable[key];
    //     if (entry.depth >= depth) {
    //         if (entry.type == 0) {
    //             return entry.score;
    //         }
    //         if (entry.type == 1 && entry.score <= a) {
    //             return a;
    //         }
    //         if (entry.type == 2 && entry.score >= b) {
    //             return b;
    //         }
    //     }
    //     validityFlag = false;
    //     return 0;
    // }

    auto negamax(
        int_fast8_t depth = 10,
        int_fast8_t colour = 1,
        int_fast16_t a = -20000,
        int_fast16_t b = 20000) -> int_fast16_t  //WORKING
    {
        assert(colour != 0);

        if (depth < 1) {
            nodes++;
            return colour * node.evaluate();
        }

        if (node.is_game_over()) {
            nodes++;
            return colour * node.evaluate() * (depth + 1);
        }

        int_fast16_t score;

        for (const auto& move : node.legal_moves()) {
            node.play(move);
            score = -negamax(depth - 1, -colour, -b, -a);
            node.unplay();

            if (score >= b)
                return b;
            if (score > a)
                a = score;
        }
        return a;
    }

    auto dnegamax(
        int_fast8_t colour = 1,
        int_fast16_t a = -20000,
        int_fast16_t b = 20000) -> int_fast16_t  //WORKING
    {
        if (node.is_game_over()) {
            nodes++;
            return colour * node.evaluate();
        }
        int_fast16_t score;

        for (const auto& move : node.legal_moves()) {
            node.play(move);
            nodes += 1;
            score = -dnegamax(-colour, -b, -a);
            node.unplay();

            if (score >= b)
                return b;
            if (score > a)
                a = score;
        }

        return a;
    }

    void engine_move()  //WORKING
    {
        Move bestmove = -1;
        int_fast16_t bestcase = -40;
        int_fast16_t score = -40;
        nodes = 0;

        auto end = std::chrono::steady_clock::now();
        end += std::chrono::milliseconds(timeLimit);

        if (GAME_SOLVABLE) {
            for (const auto& move : node.legal_moves()) {
                node.play(move);
                score = -dnegamax(node.get_turn());
                node.unplay();
                if (bestcase < score) {
                    bestcase = score;
                    bestmove = move;
                }
            }
        } else {
            int_fast8_t depth = 1;
            while (std::chrono::steady_clock::now() < end && depth < 22) {
                auto start = std::chrono::steady_clock::now();
                for (const auto move : node.legal_moves()) {
                    node.play(move);
                    score = -negamax(depth, -node.get_turn());
                    node.unplay();
                    if (bestcase < score) {
                        bestcase = score;
                        bestmove = move;
                    }
                }
                std::cout << "depth: " << (int)depth << " best move: " << (int)bestmove << " score: " << (int)bestcase << " in " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count() << "ms\n";
                depth++;
            }
        }
        std::cout << "ISTUS:\n";
        std::cout << nodes << " nodes processed.\n";
        std::cout << "Istus win prediction: " << (int)((1 + bestcase) * (50)) << "%\n";
        node.play(bestmove);
    }

    void reset_nodes() {
        nodes = 0;
    }

    void show_result() const {
        int r;
        r = node.evaluate();
        if (r == 0)
            std::cout << "1/2-1/2" << '\n';
        else if (r == 1)
            std::cout << "1-0" << '\n';
        else
            std::cout << "0-1" << '\n';
    }

    void print(const std::string input, const std::string end = "\n") const {
        std::cout << input << end;
    }

    auto get_player_move() -> Move {
        return node.get_player_move();
    }
};

class Perft {
   public:
    State node;
    int nodes = 0;

    void perftx(int n) {
        if (n == 0) {
            nodes++;
        } else {
            for (Move move : node.legal_moves()) {
                node.play(move);
                perftx(n - 1);
                node.unplay();
            }
        }
    }

    void perft(int n) {
        nodes = 0;
        perftx(n);
        std::cout << nodes;
    }
};

auto get_first_player() -> bool {
    bool player;
    std::cout << "Is the human player going first? [1/0]"
              << "\n";
    std::cin >> player;
    return player;
}

inline void run_negamax_game(const long long TL) {
    Istus glyph = Istus(TL);
    Move i;
    glyph.node.show();
    if (get_first_player()) {
        i = glyph.get_player_move();
        glyph.node.play(i);
        glyph.node.show();
    }
    while (!glyph.node.is_game_over()) {
        glyph.engine_move();
        glyph.reset_nodes();
        glyph.node.show();
        if (glyph.node.is_game_over())
            break;
        i = glyph.get_player_move();
        glyph.node.play(i);
        glyph.node.show();
    }
    glyph.show_result();
}

inline void run_mcts_game(const long long TL) {
    Zero glyph = Zero(TL);
    Move i;
    glyph.node.show();
    if (get_first_player()) {
        i = glyph.get_player_move();
        glyph.node.play(i);
        glyph.node.show();
    }
    while (!glyph.node.is_game_over()) {
        glyph.engine_move();
        glyph.searchDriver.nodes = 0;
        glyph.node.show();
        if (glyph.node.is_game_over())
            break;
        i = glyph.get_player_move();
        if (i == -1) {
            glyph.node.unplay();
            glyph.node.unplay();
            i = glyph.get_player_move();
            glyph.node.play(i);
            glyph.node.show();
        } else {
            glyph.node.play(i);
            glyph.node.show();
        }
    }
    glyph.show_result();
}

inline auto selfplay(const long long TL) -> int {
    bool ans1, ans2;
    // std::cout << "Engine 1: Zero / Istus [0/1]: ";
    // std::cin >> ans1;
    // std::cout << "Engine 2: Zero / Istus [0/1]: ";
    // std::cin >> ans2;
    Zero engine1 = Zero(TL);
    // Istus engine1i = Istus(TL);
    // Zero engine2z = Zero(TL);
    Zero engine2 = Zero(TL);
    int eturn = 1;
    while (!engine1.node.is_game_over() && !engine2.node.is_game_over()) {
        engine1.node.show();
        if (eturn == -1) {
            engine1.engine_move();
            engine2.node = engine1.node;
        } else {
            engine2.engine_move();
            engine1.node = engine2.node;
        }
        eturn = -eturn;
    }
    engine1.node.show();
    engine1.show_result();
    return engine1.node.evaluate();
}

inline void userplay() {
    Zero game = Zero();
    game.node.show();
    while (!game.node.is_game_over() && !game.node.is_game_over()) {
        int i;
        i = game.get_player_move();
        game.node.play(i);
        game.node.show();
    }
    game.node.show();
    game.show_result();
}

inline void testsuite() {
    Zero game = Zero();
    while (!game.node.is_game_over()) {
        std::cout << "\nposition legal moves: "
                  << game.node.legal_moves().size()
                  << "\nfast move counter: "
                  << game.node.num_legal_moves()
                  << "\nactual list of moves: "
                  << string(game.node.legal_moves())
                  << "\nstate of play (is game over?): "
                  << game.node.is_game_over()
                  << '\n';
        // assert(game.node.evaluate() == game.node.evaluateOLD());
        assert(game.node.legal_moves().size() == game.node.num_legal_moves());
        game.node.random_play();
    }
}

inline void benchmark() {
    const int len = 30;
    const int width = 50;
    std::array<int, len* width> nodecounts = {0};
    for (int i = 0; i < width; i++) {
        Zero engine = Zero(99);
        for (int j = 0; j < len; j++) {
            engine.engine_move();
            nodecounts[i * len + j] = engine.searchDriver.nodes;
        }
        if (engine.node.is_game_over()) {
            break;
        }
        std::cout << i << " ";
    }
    unsigned long long sum = 0;
    for (auto i : nodecounts) {
        sum += i;
    }
    std::cout << "\naverage nodecount: " << (double)sum / (50.0 * 50.0) << "\n";
}

// inline void analysis() {
//     for (int i = 0; i < 9; i++)
//     {

//     }

// }

int main() {
    // Perft p;
    // for (int i = 0; i < 10; i++)
    // {
    //     p.perft(i);
    //     std::cout << "\n";
    // }
    // return 0;

    std::cout << "Play against Zero [0] | Play against Istus [1] | Watch a self-play game [2] | Play with a friend [3] | Run tests [4] | Benchmark [5]\n--> ";
    int ans;
    std::cin >> ans;
    long long TL;
    if (ans < 3) {
        std::cout << "milliseconds per move? ";
        std::cin >> TL;
    }
    switch (ans) {
        case 0:
            run_mcts_game(TL);
            break;
        case 1:
            run_negamax_game(TL);
            break;
        case 2:
            selfplay(TL);
            break;
        case 3:
            userplay();
            break;
        case 4:
            testsuite();
            break;
        case 5:
            benchmark();
            break;
        default:
            break;
    }
    return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started:
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
