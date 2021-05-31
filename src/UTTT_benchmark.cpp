#include <chrono>
#include <iostream>
#include <limits>

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

    int rollouts = atoi(argv[1]);
    int iterations = atoi(argv[2]);

    std::cout << rollouts << " " << iterations << "\n";

    test.set_rollout_limit(rollouts);

    long long total_time = 0;
    long long min_time = std::numeric_limits<long long>::max();
    long long max_time = 0;
    for (int i = 0; i < iterations; i++) {
        test.reset_node();
        auto start = std::chrono::steady_clock::now();
        test.engine_move();
        long long time_taken = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();
        max_time = std::max(max_time, time_taken);
        min_time = std::min(min_time, time_taken);
        total_time += time_taken;
    }

    std::cout << "Min time: " << min_time << "ms\n";
    std::cout << "Max time: " << max_time << "ms\n";
    std::cout << "Avg time: " << total_time / iterations << "ms\n";

    return 0;
}
