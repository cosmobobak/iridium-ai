#include <iostream>
#include "iridium-ai.hpp"

int main() {
    std::cout << "Which game would you like to play? Checkers[1], Connect4[2], Connect4(4x4)[3], Go[4], Gomoku[5], RawTree[6], TicTacToe[7], or UTTT[8]?\n--> ";
    int response;

    std::cin >> response;
    switch (response) {
        // case Games::Checkers:
        //     main_template<Checkers::State>();
        //     break;
        case Iridium::Games::Connect4:
            Iridium::main_template<Connect4::State>();
            break;
        case Iridium::Games::Connect4x4:
            Iridium::main_template<Connect4x4::State>();
            break;
        // case Games::Go:
        //     main_template<Go::State>();
        //     break;
        case Iridium::Games::Gomoku:
            Iridium::main_template<Gomoku::State>();
            break;
        case Iridium::Games::RawTree:
            Iridium::main_template<RawTree::State>();
            break;
        case Iridium::Games::TicTacToe:
            Iridium::main_template<TicTacToe::State>();
            break;
        case Iridium::Games::UTTT:
            Iridium::main_template<UTTT::State>();
            break;

        default:
            break;
    }
    return 0;
}