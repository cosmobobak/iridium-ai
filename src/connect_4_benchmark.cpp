#include <iostream>
#include "iridium-ai.hpp"

int main(int argc, char const *argv[])
{
    if (argc <= 2) {
        std::cout << "Run with arg1: rollouts, arg2: iterations.";
        return 0;
    }
    int rollouts = atoi(argv[0]);
    int iterations = atoi(argv[1]);
    
    

    return 0;
}
