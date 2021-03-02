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

#include "MCSearch.hpp"
#include "NMSearch.hpp"

using namespace MCSearch;
using namespace NMSearch;

auto get_first_player() -> bool {
    bool player;
    std::cout << "Is the human player going first? [1/0]"
              << "\n";
    std::cin >> player;
    return player;
}

template <class GameType>
inline void run_negamax_game(const long long TL) {
    Istus<GameType> engine = Istus<GameType>(TL);
    typename GameType::Move i;
    engine.node.show();
    if (get_first_player()) {
        i = engine.get_player_move();
        engine.node.play(i);
        engine.node.show();
    }
    while (!engine.node.is_game_over()) {
        engine.engine_move();
        engine.node.show();
        if (engine.node.is_game_over())
            break;
        i = engine.get_player_move();
        engine.node.play(i);
        engine.node.show();
    }
    engine.show_result();
}

template <class GameType>
inline void run_mcts_game(const long long TL) {
    Zero<GameType> engine = Zero<GameType>(TL);
    typename GameType::Move i;
    engine.node.show();
    if (get_first_player()) {
        i = engine.get_player_move();
        engine.node.play(i);
        engine.node.show();
    }
    while (!engine.node.is_game_over()) {
        engine.engine_move();
        engine.node.show();
        if (engine.node.is_game_over())
            break;
        i = engine.get_player_move();
        if (i == -1) {
            engine.node.unplay();
            engine.node.unplay();
            i = engine.get_player_move();
            engine.node.play(i);
            engine.node.show();
        } else {
            engine.node.play(i);
            engine.node.show();
        }
    }
    engine.show_result();
}

template <class GameType>
inline auto selfplay(const long long TL) -> int {
    auto engine1 = Zero<GameType>(TL);
    auto engine2 = Zero<GameType, 7>(TL);
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

template <class GameType>
inline void userplay() {
    Zero<GameType> game = Zero<GameType>();
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

template <class GameType>
inline void testsuite() {
    Zero<GameType> game = Zero<GameType>();
    while (!game.node.is_game_over()) {
        game.node.show();
        std::cout << "\nforcing board: "
                  //<< (int)game.node.forcingBoard
                  << "\nposition legal moves: "
                  << game.node.legal_moves().size()
                  << "\nfast move counter: "
                  << (int)game.node.num_legal_moves()
                  << "\nforcing board after movegen: "
                  //<< (int)game.node.forcingBoard
                  << "\nactual list of moves: ";
        showvec(game.node.legal_moves());
        std::cout << "\nstate of play (is game over?): "
                  << game.node.is_game_over()
                  << '\n';
        // assert(game.node.evaluate() == game.node.evaluateOLD());
        assert(game.node.legal_moves().size() == game.node.num_legal_moves());
        game.node.random_play();
    }
}

template <class GameType>
inline void benchmark() {
    const int len = 30;
    const int width = 50;
    std::array<int, len* width> nodecounts = {0};
    for (int i = 0; i < width; i++) {
        Zero<GameType> engine = Zero<GameType>(99);
        for (int j = 0; j < len; j++) {
            engine.engine_move();
            nodecounts[i * len + j] = engine.searchDriver.get_nodes();
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

template <class GameType>
void main_template() {
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
            run_mcts_game<GameType>(TL);
            break;
        case 1:
            run_negamax_game<GameType>(TL);
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

int main() {
    std::cout << "Which game would you like to play? Checkers[1], Connect4[2], Connect4(4x4)[3], Go[4], Gomoku[5], RawTree[6], TicTacToe[7], or UTTT[8]?\n--> ";
    int response;
    
    std::cin >> response;
    switch (response) {
        // case Games::Checkers:
        //     main_template<Checkers::State>();
        //     break;
        case Games::Connect4:
            main_template<Connect4::State>();
            break;
        case Games::Connect4x4:
            main_template<Connect4x4::State>();
            break;
        // case Games::Go:
        //     main_template<Go::State>();
        //     break;
        case Games::Gomoku:
            main_template<Gomoku::State>();
            break;
        case Games::RawTree:
            main_template<RawTree::State>();
            break;
        case Games::TicTacToe:
            main_template<TicTacToe::State>();
            break;
        case Games::UTTT:
            main_template<UTTT::State>();
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
