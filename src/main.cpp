#include <iostream>

#include "iridium-ai.hpp"

using namespace Iridium;

auto main() -> int {
    std::cout << "Which game would you like to play? Checkers[1], Connect4[2], Connect4(4x4)[3], Go[4], Gomoku[5], TicTacToe[6], or UTTT[7]?\n--> ";

    auto response = 0;
    std::cin >> response;

    switch ((Game)response) {
        case Game::Connect4:
            main_template<Connect4::State<6, 7>>();
            break;
        case Game::Connect4x4:
            main_template<Connect4x4::State>();
            break;
        case Game::Gomoku:
            main_template<Gomoku::State<15,15>>();
            break;
        case Game::TicTacToe:
            main_template<TicTacToe::State>();
            break;
        case Game::UTTT:
            main_template<UTTT::State>();
            break;
        // case Game::Go:
        //     main_template<Go::State>();
        //     break;
        // case Game::Checkers:
        //     main_template<Checkers::State>();
        //     break;

        default:
            break;
    }
    return 0;
}