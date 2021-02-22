cmake_minimum_required(VERSION 3.17)
project(iridium_ai)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)

add_executable(iridium_ai
        accelerations.hpp
        Checkers.hpp
        Connect4-4x4.hpp
        Connect4.hpp
        Go.hpp
        Gomoku.hpp
        iridium-ai.cpp
        RawTree.hpp
        TicTacToe.hpp
        UTTT.hpp)
