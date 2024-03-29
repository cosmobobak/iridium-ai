#pragma once

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

#include "Zero.hpp"
#include "Istus.hpp"

namespace Iridium {
auto get_first_player() -> bool {
    bool player;
    std::cout << "Is the human player going first? [1/0]"
              << "\n";
    std::cin >> player;
    return player;
}

template <class State>
void run_negamax_engine(const long long TL) {
    auto engine = Istus<State>(TL);
    typename State::Move i;
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

template <class State>
void run_mcts_engine(const long long TL) {
    auto engine = Zero<State>(TL);
    engine.set_debug(false);
    engine.set_readout(true);
    engine.use_time_limit(true);
    engine.set_time_limit(TL);
    typename State::Move i;
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

template <class State>
auto selfplay(const long long TL) -> int {
    auto engine1 = Zero<State>(TL);
    auto engine2 = Zero<State>(TL);
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
    size_t rounds;
    std::cin >> rounds;

    std::cout << "generation should be done in about " << ((double)rounds * 30.0 * (350.0 / 50000.0) * rollouts) / (1000.0 * 60.0 * 60.0) << " hours. good luck.\n";

    std::vector<int> results(rounds);
    for (size_t i = 0; i < rounds; i++) {
        engine1.set_node(State());
        engine2.set_node(State());
        /* code */
        int engine_turn = 1;
        while (!engine1.is_game_over() && !engine2.is_game_over()) {
            engine1.show_node();
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

template <class State>
void userplay() {
    auto engine = Zero<State>();
    engine.get_node().show();
    int i;
    while (!engine.get_node().is_game_over() && !engine.get_node().is_game_over()) {
        i = engine.get_player_move();
        engine.get_node().play(i);
        engine.get_node().show();
    }
    engine.get_node().show();
    engine.show_result();
}

template <class State>
void testsuite() {
    auto engine = Zero<State>();
    engine.get_node().show();
    std::cout << "\nforcing board: "
              //<< (int)engine.get_node().forcingBoard
              << "\nposition legal moves: "
              << engine.get_node().legal_moves().size()
              << "\nfast move counter: "
              << (int)engine.get_node().num_legal_moves()
              << "\nforcing board after movegen: "
              //<< (int)engine.get_node().forcingBoard
              << "\nactual list of moves: \n";
    for (auto move : engine.get_node().legal_moves()) {
        std::cout << (int)move << " ";
    }
    std::cout << "\nstate of play (is engine over?): "
              << engine.get_node().is_game_over()
              << '\n';
    // assert(engine.get_node().evaluate() == engine.get_node().evaluateOLD());
    assert(engine.get_node().legal_moves().size() == engine.get_node().num_legal_moves());
    while (!engine.get_node().is_game_over()) {
        engine.get_node().random_play();
        engine.get_node().show();
        std::cout << "\nforcing board: "
                  //<< (int)engine.get_node().forcingBoard
                  << "\nposition legal moves: "
                  << engine.get_node().legal_moves().size()
                  << "\nfast move counter: "
                  << (int)engine.get_node().num_legal_moves()
                  << "\nforcing board after movegen: "
                  //<< (int)engine.get_node().forcingBoard
                  << "\nactual list of moves: \n";
        for (auto move : engine.get_node().legal_moves()) {
            std::cout << (int)move << " ";
        }
        std::cout << "\nstate of play (is engine over?): "
                  << engine.get_node().is_game_over()
                  << '\n';
        // assert(engine.get_node().evaluate() == engine.get_node().evaluateOLD());
        assert(engine.get_node().legal_moves().size() == engine.get_node().num_legal_moves());
    }
}

template <class State>
void benchmark() {
    const int len = 30;
    const int width = 50;
    std::vector<int> nodecounts(len * width);
    for (int i = 0; i < width; i++) {
        auto engine = Zero<State>(99);
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

template <class State>
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
            run_mcts_engine<State>(TL);
            break;
        case 1:
            run_negamax_engine<State>(TL);
            break;
        case 2:
            selfplay<State>(TL);
            break;
        case 3:
            userplay<State>();
            break;
        case 4:
            testsuite<State>();
            break;
        case 5:
            benchmark<State>();
            break;
        default:
            break;
    }
}

enum class Game : int {
    Checkers = 1,
    Connect4,
    Connect4x4,
    Go,
    Gomoku,
    TicTacToe,
    UTTT
};

}  // namespace Iridium
