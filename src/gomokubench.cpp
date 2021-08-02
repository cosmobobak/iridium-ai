#include "bench.hpp"
#include "games/Gomoku.hpp"

int main(int argc, char const *argv[]) {
    if (argc <= 2) {
        std::cout << "Run with arg1: rollouts, arg2: iterations.\n";
        return 0;
    }

    auto rollouts = atoi(argv[1]);
    auto iterations = atoi(argv[2]);

    printf("%d %d\n", rollouts, iterations);

    bench::benchmark<Gomoku::State<8, 8>>(rollouts, iterations);

    return 0;
}