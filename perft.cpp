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

// #include "Checkers.hpp"
#include "Connect4-4x4.hpp"
#include "Connect4.hpp"
// #include "Go.hpp"
#include "Gomoku.hpp"
#include "RawTree.hpp"
#include "TicTacToe.hpp"
#include "UTTT.hpp"

template <class T, typename Move>
class Perft {
   public:
    T node;
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
        std::cout << nodes << "\n";
    }
};

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

int main(int argc, char const *argv[]) {
    std::cout << "Perft movegen checker: would you like to check Checkers[1], Connect4[2], Connect4(4x4)[3], Go[4], Gomoku[5], RawTree[6], TicTacToe[7], or UTTT[8]?\n--> ";
    int response;
    std::cin >> response;

    // Perft<Checkers::State> engine1;
    Perft<Connect4::State, Connect4::Move> engine2;
    Perft<Connect4x4::State, Connect4x4::Move> engine3;
    // Perft<Go::State> engine4;
    Perft<Gomoku::State, Gomoku::Move> engine5;
    Perft<RawTree::State, RawTree::Move> engine6;
    Perft<TicTacToe::State, TicTacToe::Move> engine7;
    Perft<UTTT::State, UTTT::Move> engine8;

    switch (response) {
        // case Games::Checkers:
        //     for (int i = 0; i < 10; i++) {
        //         engine1.perft(i);
        //     }
        //     break;
        case Games::Connect4:
            for (int i = 0; i < 10; i++) {
                engine2.perft(i);
            }
            break;
        case Games::Connect4x4:
            for (int i = 0; i < 10; i++) {
                engine3.perft(i);
            }
            break;
        // case Games::Go:
        //     for (int i = 0; i < 4; i++) {
        //         engine4.perft(i);
        //     }
        //     break;
        case Games::Gomoku:
            for (int i = 0; i < 10; i++) {
                engine5.perft(i);
            }
            break;
        case Games::RawTree:
            for (int i = 0; i < 2; i++) {
                engine6.perft(i);
            }
            break;
        case Games::TicTacToe:
            for (int i = 0; i < 9; i++) {
                engine7.perft(i);
            }
            break;
        case Games::UTTT:
            for (int i = 0; i < 10; i++) {
                engine8.perft(i);
            }
            break;

        default:
            break;
    }
    return 0;
}
