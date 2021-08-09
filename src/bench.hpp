#pragma once

#include <chrono>
#include <iostream>
#include <limits>
#include <cstdio>

#include "games/UTTT2.hpp"
#include "Zero.hpp"

namespace std {
    using namespace chrono;
}

namespace bench {

template <typename ST>
[[nodiscard]] auto get_benchmark_engine() {
    auto out = Zero<ST>();
    out.use_rollout_limit(true);
    out.set_readout(false);
    out.set_debug(false);
    return out;
}

template <typename ST>
void benchmark(int rollouts, int iterations) {
    auto engine = get_benchmark_engine<ST>();
    engine.set_rollout_limit(rollouts);

    auto total_time = 0LL;
    auto min_time = std::numeric_limits<long long>::max();
    auto max_time = 0LL;
    for (int i = 0; i < iterations; ++i) {
        printf("%d%% done!\r", i * 100 / iterations);
        fflush(stdout);

        engine.reset_node();
        auto start = std::steady_clock::now();
        engine.engine_move();
        auto end = std::steady_clock::now();

        auto time_taken = (long long)std::duration_cast<std::microseconds>(end - start).count();
        max_time = std::max(max_time, time_taken);
        min_time = std::min(min_time, time_taken);
        total_time += time_taken;
    }
    printf("%d%% done!\n", 100);

    printf("Min time: %lldus\n", min_time);
    printf("Max time: %lldus\n", max_time);
    printf("Avg time: %lldus\n", total_time / iterations);
}
}