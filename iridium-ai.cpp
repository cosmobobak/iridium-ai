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

using namespace TicTacToe;

using namespace MCSearch;
using namespace NMSearch;

auto get_first_player() -> bool {
    bool player;
    std::cout << "Is the human player going first? [1/0]"
              << "\n";
    std::cin >> player;
    return player;
}

inline void run_negamax_game(const long long TL) {
    Istus engine = Istus(TL);
    Move i;
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

inline void run_mcts_game(const long long TL) {
    Zero engine = Zero(TL);
    Move i;
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

inline auto selfplay(const long long TL) -> int {
    Zero engine1 = Zero(TL);
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

inline void benchmark() {
    const int len = 30;
    const int width = 50;
    std::array<int, len* width> nodecounts = {0};
    for (int i = 0; i < width; i++) {
        Zero engine = Zero(99);
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
