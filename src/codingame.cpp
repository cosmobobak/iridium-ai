#pragma GCC optimize("Ofast", "unroll-loops", "omit-frame-pointer", "inline")
#pragma GCC target("avx")

#include <algorithm>
#include <array>
#include <chrono>
#include <climits>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <vector>

template <typename T>
void showvec(std::vector<T> v) {
    std::cerr << "{ ";
    for (auto&& i : v) {
        std::cerr << (int)i;
        std::cerr << ", ";
    }
    std::cerr << "}";
}

inline auto popcount(const int_fast16_t bb) -> int_fast8_t {
    return __builtin_popcount(bb);
}

inline auto lsb(int_fast16_t bitboard) -> int_fast8_t {
    return __builtin_ctz(bitboard);
}

inline void copy_int8_array_81(const int_fast8_t in[], int_fast8_t out[]) {
    for (int_fast8_t i = 0; i < 81; i++) {
        out[i] = in[i];
    }
}

namespace MOD9 {
constexpr uint_fast8_t LOOKUP[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 0, 1, 2, 3, 4, 5, 6, 7, 8, 0, 1, 2, 3, 4, 5, 6, 7, 8, 0, 1, 2, 3, 4, 5, 6, 7, 8, 0, 1, 2, 3, 4, 5, 6, 7, 8, 0, 1, 2, 3, 4, 5, 6, 7, 8, 0, 1, 2, 3, 4, 5, 6, 7, 8, 0, 1, 2, 3, 4, 5, 6, 7, 8, 0, 1, 2, 3, 4, 5, 6, 7, 8};
}

namespace Board {

using Move = uint_fast8_t;
using Bitboard = uint_fast16_t;
using Num = int_fast8_t;

class SubState {
   public:
    Bitboard position[2] = {0b000000000, 0b000000000};
    // bool cached_death_state = false;

    void reset() {
        position[0] = 0b000000000;
        position[1] = 0b000000000;
    }

    inline auto union_bb() const -> Bitboard {
        return position[0] | position[1];
    }

    inline void play(const Num i, const Num idx) {
        // n ^ (1 << k) is a binary XOR where you flip the kth bit of n
        position[idx] ^= (1 << i);
    }

    inline void unplay(const Num prevmove, const Num idx)  // do not unplay on root
    {
        position[idx] ^= (1 << prevmove);
    }

    inline auto pos_filled(const Num i) const -> bool {
        return position[0] & (1 << i) || position[1] & (1 << i);
    }

    inline auto player_at(const Num i) const -> bool  //only valid to use if pos_filled() returns true, true = x, false = y
    {
        return (position[0] & (1 << i));
    }

    inline auto is_full() const -> bool {
        return position[0] + position[1] == 0b111111111;
    }

    inline auto evaluate() const -> Num {
        // check first diagonal
        if (pos_filled(0) && pos_filled(4) && pos_filled(8)) {
            if (player_at(0) == player_at(4) && player_at(4) == player_at(8)) {
                if (player_at(0))
                    return 1;
                else
                    return -1;
            }
        }
        // check second diagonal
        if (pos_filled(2) && pos_filled(4) && pos_filled(6)) {
            if (player_at(2) == player_at(4) && player_at(4) == player_at(6)) {
                if (player_at(2))
                    return 1;
                else
                    return -1;
            }
        }
        // check rows
        for (Num i = 0; i < 3; i++) {
            if (pos_filled(i * 3) && pos_filled(i * 3 + 1) && pos_filled(i * 3 + 2)) {
                if (player_at(i * 3) == player_at(i * 3 + 1) && player_at(i * 3 + 1) == player_at(i * 3 + 2)) {
                    if (player_at(i * 3))
                        return 1;
                    else
                        return -1;
                }
            }
        }
        // check columns
        for (Num i = 0; i < 3; i++) {
            if (pos_filled(i) && pos_filled(i + 3) && pos_filled(i + 6)) {
                if (player_at(i) == player_at(i + 3) && player_at(i + 3) == player_at(i + 6)) {
                    if (player_at(i))
                        return 1;
                    else
                        return -1;
                }
            }
        }
        return 0;
    }

    inline auto is_board_dead() const -> bool {
        return is_full() || evaluate();
    }
};
}  // namespace Board

namespace UTTT {

constexpr auto GAME_SOLVABLE = false;
constexpr auto gameexpfactor = 5;

class State {
   public:
    using Move = uint_fast16_t;
    using Bitboard = uint_fast16_t;
    using Num = int_fast8_t;
    std::array<Board::SubState, 9> metaposition;
    Num forcingBoard = -1;
    Num movecount = 0;
    std::vector<Num> movestack;     // reserve for the playout boards
    std::vector<Num> forcingstack;  // reserve for the playout boards

    State() {
        movestack.reserve(16);
        forcingstack.reserve(16);
        for (Board::SubState& board : metaposition) {
            board = Board::SubState();
        }
        forcingstack.push_back(-1);
    }
    State(const UTTT::State& inputState) {
        metaposition = inputState.metaposition;
        forcingBoard = inputState.forcingBoard;
        movecount = inputState.movecount;
        forcingstack = inputState.forcingstack;
    }

    inline void mem_setup() {
        movestack.reserve(81);
        forcingstack.reserve(81);
    }

    void reset() {
        std::for_each(
            metaposition.begin(),
            metaposition.end(),
            [](Board::SubState& b) { b.reset(); });
    }

    inline void play(const Num i) {
        const Num board = i / 9;
        const Num square = MOD9::LOOKUP[i];
        metaposition[board].play(square, movecount & 1);
        movestack.push_back(i);
        movecount++;
        forcingBoard = square;
        forcingstack.push_back(forcingBoard);
    }

    inline void unplay()  // do not unplay on root
    {
        movecount--;
        const Num prevmove = movestack.back();
        const Num board = prevmove / 9;
        const Num square = MOD9::LOOKUP[prevmove];
        movestack.pop_back();
        metaposition[board].unplay(square, movecount & 1);
        forcingstack.pop_back();
        forcingBoard = forcingstack.back();
    }

    inline auto board_won(const Num board) const -> bool {
        return metaposition[board].evaluate() != 0;
    }

    inline auto board_over(const Num board) const -> bool {
        return metaposition[board].is_board_dead();
    }

    inline auto winner_of_board(const Num board) const -> int_fast8_t  //only valid to use if pos_filled() returns true, true = x, false = y
    {
        return metaposition[board].evaluate();
    }

    inline auto is_full() -> bool {
        return std::all_of(metaposition.begin(), metaposition.end(), [](Board::SubState p) { return p.is_board_dead(); });
    }

    inline auto evaluate() const -> Num {
        // check first diagonal
        Num middle = winner_of_board(4);
        if (middle) {
            if (winner_of_board(0) == middle && middle == winner_of_board(8)) {
                return middle;
            }
            if (winner_of_board(2) == middle && middle == winner_of_board(6)) {
                return middle;
            }
        }

        // check second diagonal

        // check rows
        for (Num i = 0; i < 3; i++) {
            if (winner_of_board(i * 3)) {
                if (winner_of_board(i * 3) == winner_of_board(i * 3 + 1) && winner_of_board(i * 3 + 1) == winner_of_board(i * 3 + 2)) {
                    return winner_of_board(i * 3);
                }
            }
        }
        // check columns
        for (Num i = 0; i < 3; i++) {
            if (winner_of_board(i)) {
                if (winner_of_board(i) == winner_of_board(i + 3) && winner_of_board(i + 3) == winner_of_board(i + 6)) {
                    return winner_of_board(i);
                }
            }
        }
        Num count = 0;
        for (Num i = 0; i < 9; i++) {
            if (board_over(i)) {
                count += winner_of_board(i);
            } else {
                return 0;
            }
        }
        if (count > 0) {
            return 1;
        } else {
            return -1;
        }
    }

    inline void pass_turn() {
        movecount++;
    }

    auto get_turn() const -> uint_fast8_t {
        if (movecount & 1)
            return -1;
        else
            return 1;
    }

    inline auto is_game_over() const -> bool {
        if (num_legal_moves() == 0)
            return true;
        return (evaluate() != 0);
    }

    auto num_legal_moves() const -> Num {
        if (forcingBoard != -1)
            return 9 - __builtin_popcount(metaposition[forcingBoard].union_bb());
        Num cnt = 0;
        for (Num i = 0; i < 9; i++) {
            if (!metaposition[i].is_board_dead())
                cnt += 9 - __builtin_popcount(metaposition[i].union_bb());
        }
        return cnt;
    }

    inline auto legal_moves() -> std::vector<Move> {
        std::vector<Move> moves;
        if (metaposition[forcingBoard].is_board_dead())
            forcingBoard = -1;
        if (forcingBoard == -1) {
            moves.reserve(81);
            Num bcounter = 0;
            for (const auto& board : metaposition) {
                if (!board.is_board_dead()) {
                    uint_fast16_t bb = ~board.union_bb() & 0b111111111;
                    while (bb) {
                        moves.push_back(bcounter * 9 + (__builtin_ctz(bb)));
                        bb &= bb - 1;  // clear the least significant bit set
                    }
                }
                bcounter++;
            }
        } else {
            moves.reserve(9);
            Bitboard bb = ~metaposition[forcingBoard].union_bb() & 0b111111111;
            while (bb) {
                moves.push_back(forcingBoard * 9 + (__builtin_ctz(bb)));
                bb &= bb - 1;  // clear the least significant bit set
            }
        }
        return moves;
    }

    inline void random_play() {
        std::vector<Move> moves = legal_moves();
        play(moves[rand() % moves.size()]);
    }

    inline auto heuristic_value() const -> uint_fast8_t {
        return rand() & 0b1111;
    }
};

bool operator==(const Board::SubState& a, const Board::SubState& b) {
    return a.position == b.position;
}

bool operator==(const UTTT::State& a, const UTTT::State& b) {
    if (a.forcingBoard != b.forcingBoard)
        return false;
    for (short i = 0; i < 9; i++) {
        if (a.metaposition[i].position != b.metaposition[i].position)
            return false;
    }
    return true;
}
}  // namespace UTTT

using namespace UTTT;

namespace TreeNode {
template <class ContainedState>
class Node {
   public:
    int_fast32_t winScore = 0;
    uint_fast32_t visits = 0;
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
};
}  // namespace TreeNode

constexpr auto EXP_FACTOR = gameexpfactor;

using namespace TreeNode;

template <class NodeType, int EXP_FACTOR>
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

namespace SearchDriver {
constexpr uint_fast8_t REWARD = 10;

template <class StateType>
class MCTS {
    long long time_limit;        // limiter on search time
    const bool memsafe = true;  // dictates whether we preserve a part of the tree across moves
    uint_fast8_t opponent;      // the win score that the opponent wants
    int_fast32_t node_count = 0;
    Node<StateType>* preservedNode = nullptr;

   public:
    MCTS() {
        MCTS(1);
    }
    MCTS(const int_fast8_t player) {
        MCTS(player, 3);
    }
    MCTS(const int_fast8_t player, const long long strength) {
        srand(time(NULL));
        time_limit = strength;
        opponent = -player;
    }

    void set_time_limit(long long tl) {
        time_limit = tl;
    }

    void set_nodes(int_fast32_t n) {
        node_count = n;
    }

    auto get_nodes() -> int_fast32_t {
        return node_count;
    }

    void set_opponent(const int_fast8_t i) {
        opponent = i;
    }

    void deleteTree(Node<StateType>* root) {
        /* first delete the subtrees */
        for (Node<StateType>* child : root->children) {
            deleteTree(child);
        }
        /* then delete the node */
        delete root;
    }

    auto prune(Node<StateType>* parent, const StateType& target) -> Node<StateType>* {
        Node<StateType>* out = nullptr;
        bool found = false;
        for (Node<StateType>* child : parent->get_children()) {
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

    auto find_best_next_board(const StateType board) -> StateType {
        node_count = 0;
        set_opponent(-board.get_turn());
        // an end time which will act as a terminating condition
        auto end = std::chrono::steady_clock::now() + std::chrono::milliseconds(time_limit);
        Node<StateType>* rootNode = nullptr;
        if (preservedNode) {
            rootNode = prune(preservedNode, board);
        }
        if (!rootNode) {
            rootNode = new Node<StateType>(board);
            rootNode->set_state(board);
            rootNode->set_player_no(opponent);
        }
        // if you start getting weird out_of_range() errors at low TC then expand the root node here
        // auto breakunit = std::chrono::steady_clock::now();
        while (std::chrono::steady_clock::now() < end) {
            Node<StateType>* promisingNode = select_promising_node(rootNode);

            if (!promisingNode->board.is_game_over())
                expand_node(promisingNode);

            Node<StateType>* nodeToExplore = promisingNode;

            if (promisingNode->get_children().size() != 0)
                nodeToExplore = promisingNode->random_child();

            uint_fast8_t result = simulate_playout(nodeToExplore);
            backprop(nodeToExplore, result);

            // if (std::chrono::steady_clock::now() > breakunit) {
            //     std::cout << "The best move so far is: " << rootNode->best_child_as_move() + 1 << '\n';
            //     breakunit += std::chrono::milliseconds(500);
            //     rootNode->show_child_visitrates();
            // }
            node_count++;
        }
        StateType out = rootNode->best_child()->get_state();
        // std::cout << "ZERO:\n";
        std::cout << node_count << " nodes processed at " << (double)node_count / ((double)time_limit / 1000.0) << "NPS.\n";
        // std::cout << nodes << ", ";
        // std::cout << "Zero win prediction: " << (int)(rootNode->best_child()->get_winrate() * (100 / WIN_SCORE)) << "%\n";
        uint_fast8_t action, sboard, square, row, col;
        action = rootNode->best_child_as_move();
        // assert(action >= 0 && action <= 80);
        // std::cout << action << '\n';
        square = action % 9;
        sboard = action / 9;
        col = (sboard % 3) * 3 + square % 3;
        row = (sboard / 3) * 3 + square / 3;
        std::cout << row << " " << col << std::endl;
        // rootNode->show_child_visitrates();
        if (!memsafe) {
            deleteTree(rootNode);
        } else {
            preservedNode = prune(rootNode, out);
        }
        return out;
    }

    inline auto select_promising_node(Node<StateType>* const rootNode) -> Node<StateType>* {
        Node<StateType>* node = rootNode;
        while (node->get_children().size() != 0)
            node = UCT<Node<StateType>, 6>::best_node_uct(node);
        return node;
    }

    inline void expand_node(Node<StateType>* node) {
        node->expand();
    }

    inline void backprop(Node<StateType>* nodeToExplore, const int_fast8_t winner) {
        Node<StateType>* propagator = nodeToExplore;
        while (propagator) {
            propagator->increment_visits();
            if (propagator->get_player_no() == winner) {
                propagator->add_score(REWARD);
            }
            propagator = propagator->get_parent();
        }
    }

    inline auto simulate_playout(Node<StateType>* node) -> int_fast8_t {
        StateType tempState = node->get_state();
        tempState.mem_setup();
        uint_fast8_t status = tempState.evaluate();
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
};  // namespace SearchDriver

// Possible heuristic improvement: use a long search to generate MCTS values for each starting square, use them as a heuristic starter.
// The RAVE approach makes this heuristic value = some sort of aggregate score of the move on parent nodes.
// UCT becomes (simulation value / rollouts) + (heuristic value / rollouts) + (exploration factor)

namespace MCSearch {

template <class StateType>
class Zero {
   public:
    SearchDriver::MCTS<StateType> searchDriver = SearchDriver::MCTS<StateType>();
    StateType node = StateType();

    Zero() {
        Zero(99);
    }
    Zero(const long long strength) {
        searchDriver.set_time_limit(strength);
    }

    inline void print(const std::string input, const std::string end = "\n") {
        std::cout << input << end;
    }

    auto get_player_move() {
        return node.get_player_move();
    }

    void engine_move() {
        node = searchDriver.find_best_next_board(node);
    }
};
}  // namespace MCSearch

// Possible heuristic improvement: use a long search to generate MCTS values for each starting square, use them as a heuristic starter.
// The RAVE approach makes this heuristic value = some sort of aggregate score of the move on parent nodes.
// UCT becomes (simulation value / rollouts) + (heuristic value / rollouts) + (exploration factor)

/**
 * Auto-generated code below aims at helping you parse
 * the standard input according to the problem statement.
 **/

using namespace MCSearch;

int main() {
    Zero<State> agent = Zero<State>(95);
    // game loop
    short numMovesIn = 0;
    while (1) {
        int opponentRow = -1;
        int opponentCol = -1;
        std::cin >> opponentRow >> opponentCol;
        std::cin.ignore();
        int validActionCount;
        std::cin >> validActionCount;
        std::cin.ignore();
        for (int i = 0; i < validActionCount; i++) {
            int row;
            int col;
            std::cin >> row >> col;
            std::cin.ignore();
        }
        int action, board, square;
        board = (opponentRow / 3) * 3 + (opponentCol / 3);
        square = (opponentRow % 3) * 3 + (opponentCol % 3);
        action = board * 9 + square;
        // Write an action using cout. DON'T FORGET THE "<< endl"
        // To debug: cerr << "Debug messages..." << endl;
        if (opponentRow != -1) {
            agent.node.play(action);  // we're going first
            numMovesIn++;
        }
        if (numMovesIn == 0) {
            agent.node.play(40);
            std::cout << "4 4" << std::endl;
        } else {
            agent.engine_move();
        }
        numMovesIn++;
    }
}