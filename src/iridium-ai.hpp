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

#include "MCSearch.hpp"
#include "NMSearch.hpp"

using namespace MCSearch;
using namespace NMSearch;

namespace Iridium {
auto get_first_player() -> bool {
    bool player;
    std::cout << "Is the human player going first? [1/0]"
              << "\n";
    std::cin >> player;
    return player;
}

template <class GameType>
void run_negamax_engine(const long long TL) {
    Istus<GameType> engine = Istus<GameType>(TL);
    typename GameType::Move i;
    engine.get_node().show();
    if (get_first_player()) {
        i = engine.get_player_move();
        engine.get_node().play(i);
        engine.get_node().show();
    }
    while (!engine.get_node().is_game_over()) {
        engine.engine_move();
        engine.get_node().show();
        if (engine.get_node().is_game_over())
            break;
        i = engine.get_player_move();
        engine.get_node().play(i);
        engine.get_node().show();
    }
    engine.show_result();
}

template <class GameType, int UCT_EXP_FACTOR>
void run_mcts_engine(const long long TL) {
    Zero<GameType, UCT_EXP_FACTOR> engine = Zero<GameType, UCT_EXP_FACTOR>(TL);
    engine.set_debug(false);
    engine.use_time_limit(true);
    engine.set_time_limit(TL);
    typename GameType::Move i;
    engine.get_node().show();
    if (get_first_player()) {
        i = engine.get_player_move();
        engine.get_node().play(i);
        engine.get_node().show();
    }
    while (!engine.get_node().is_game_over()) {
        engine.engine_move();
        engine.get_node().show();
        if (engine.get_node().is_game_over())
            break;
        i = engine.get_player_move();
        if (i == -1) {
            engine.get_node().unplay();
            engine.get_node().unplay();
            i = engine.get_player_move();
            engine.get_node().play(i);
            engine.get_node().show();
        } else {
            engine.get_node().play(i);
            engine.get_node().show();
        }
    }
    engine.show_result();
}

template <class GameType>
auto selfplay(const long long TL) -> int {
    auto engine1 = Zero<GameType, 10>(TL);
    auto engine2 = Zero<GameType, 8>(TL);
    engine1.use_rollout_limit(true);
    engine2.use_rollout_limit(true);
    engine1.set_readout(false);
    engine1.set_debug(false);
    engine2.set_readout(false);
    engine2.set_debug(false);

    std::cout << "Enter the number of rollouts/move:\n--> ";
    int rollouts;
    std::cin >> rollouts;
    engine1.set_rollout_limit(rollouts);
    engine2.set_rollout_limit(rollouts);

    std::cout << "Enter the number of games to play:\n--> ";
    int rounds;
    std::cin >> rounds;

    std::cout << "generation should be done in about " << ((double)rounds * 30.0 * (350.0 / 50000.0) * rollouts) / (1000.0 * 60.0 * 60.0) << " hours. good luck.\n";

    std::vector<int> results(rounds);
    for (size_t i = 0; i < rounds; i++) {
        engine1.set_node(GameType());
        engine2.set_node(GameType());
        /* code */
        int engine_turn = 1;
        while (!engine1.is_game_over() && !engine2.is_game_over()) {
            // engine1.show_node();
            if (engine_turn == -1) {
                engine1.engine_move();
                engine2.set_node(engine1.get_node());
            } else {
                engine2.engine_move();
                engine1.set_node(engine2.get_node());
            }
            engine_turn = -engine_turn;
        }
        engine1.show_node();
        engine1.show_result();
        results[i] = engine1.get_node_eval();
    }
    int p1win = std::count_if(results.begin(), results.end(), [](int i){ return i == 1; });
    int p2win = std::count_if(results.begin(), results.end(), [](int i) { return i == -1; });
    int draw = std::count_if(results.begin(), results.end(), [](int i) { return i == 0; });
    std::cout << "\n wins: " << p1win << " draws: " << draw << " losses: " << p2win << "\n";
    return 1;
}

template <class GameType>
void userplay() {
    Zero<GameType, 8> engine = Zero<GameType, 8>();
    engine.get_node().show();
    while (!engine.get_node().is_game_over() && !engine.get_node().is_game_over()) {
        int i;
        i = engine.get_player_move();
        engine.get_node().play(i);
        engine.get_node().show();
    }
    engine.get_node().show();
    engine.show_result();
}

template <class GameType>
void testsuite() {
    Zero<GameType, 8> engine = Zero<GameType, 8>();
    while (!engine.get_node().is_game_over()) {
        engine.get_node().show();
        std::cout << "\nforcing board: "
                  //<< (int)engine.get_node().forcingBoard
                  << "\nposition legal moves: "
                  << engine.get_node().legal_moves().size()
                  << "\nfast move counter: "
                  << (int)engine.get_node().num_legal_moves()
                  << "\nforcing board after movegen: "
                  //<< (int)engine.get_node().forcingBoard
                  << "\nactual list of moves: ";
        showvec(engine.get_node().legal_moves());
        std::cout << "\nstate of play (is engine over?): "
                  << engine.get_node().is_game_over()
                  << '\n';
        // assert(engine.get_node().evaluate() == engine.get_node().evaluateOLD());
        assert(engine.get_node().legal_moves().size() == engine.get_node().num_legal_moves());
        engine.get_node().random_play();
    }
}

template <class GameType>
void benchmark() {
    const int len = 30;
    const int width = 50;
    std::vector<int> nodecounts(len * width);
    for (int i = 0; i < width; i++) {
        Zero<GameType, 8> engine = Zero<GameType, 8>(99);
        engine.set_debug(false);
        engine.set_readout(false);
        for (int j = 0; j < len && !engine.get_node().is_game_over(); j++) {
            engine.engine_move();
            nodecounts[i * len + j] = engine.get_node_count();
        }
        std::cout << i << " ";
    }
    unsigned long long sum = 0;
    for (auto i : nodecounts) {
        sum += i;
    }
    std::cout << "\naverage nodecount: " << (double)sum / (50.0 * 50.0) << "\n";
}

template <class GameType, int UCT_EXP_FACTOR>
void main_template() {
    std::cout << "Play against Zero [0] | Play against Istus [1] | Watch a self-play engine [2] | Play with a friend [3] | Run tests [4] | Benchmark [5]\n--> ";
    int ans;
    std::cin >> ans;
    long long TL;
    if (ans < 3) {
        std::cout << "milliseconds per move? ";
        std::cin >> TL;
    }
    switch (ans) {
        case 0:
            run_mcts_engine<GameType, UCT_EXP_FACTOR>(TL);
            break;
        case 1:
            run_negamax_engine<GameType>(TL);
            break;
        case 2:
            selfplay<GameType>(TL);
            break;
        case 3:
            userplay<GameType>();
            break;
        case 4:
            testsuite<GameType>();
            break;
        case 5:
            benchmark<GameType>();
            break;
        default:
            break;
    }
}

namespace Games {
enum names : int {
    Checkers = 1,
    Connect4,
    Connect4x4,
    Go,
    Gomoku,
    RawTree,
    TicTacToe,
    UTTT
};
}

}  // namespace Iridium
