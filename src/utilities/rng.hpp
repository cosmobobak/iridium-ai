#pragma once

#include <random>

namespace rng {

static auto gen = std::ranlux24(std::chrono::steady_clock::now().time_since_epoch().count());

inline auto random_int(size_t range_size) {
    return gen() % range_size;
}

template <typename T>
auto choice(const std::vector<T>& vec) -> T {
    return vec[random_int(vec.size())];
}

} // namespace rng