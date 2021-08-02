#include <chrono>
#include <iostream>
#include <limits>
#include <cstdio>

#include "games/UTTT2.hpp"
#include "Zero.hpp"

int main(int argc, char const *argv[]) {
    auto test = Zero<UTTT::State>();
    test.use_rollout_limit(true);
    test.set_readout(false);
    test.set_debug(false);

    if (argc <= 2) {
        test.set_rollout_limit(200000);
        test.engine_move();
        std::cout << "Run with arg1: rollouts, arg2: iterations.\n";
        return 0;
    }

    auto rollouts = atoi(argv[1]);
    auto iterations = atoi(argv[2]);

    printf("%d %d\n", rollouts, iterations);

    test.set_rollout_limit(rollouts);

    long long total_time = 0;
    long long min_time = std::numeric_limits<long long>::max();
    long long max_time = 0;
    for (int i = 0; i < iterations; i++) {
        printf("%d%% done!\r", i * 100 / iterations);
        // flush
        fflush(stdout);
        test.reset_node();
        auto start = std::chrono::steady_clock::now();
        test.engine_move();
        long long time_taken = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - start).count();
        max_time = std::max(max_time, time_taken);
        min_time = std::min(min_time, time_taken);
        total_time += time_taken;
    }
    printf("%d%% done!\n", 100);

    printf("Min time: %lldus\n", min_time);
    printf("Max time: %lldus\n", max_time);
    printf("Avg time: %lldus\n", total_time / iterations);

    return 0;
}
