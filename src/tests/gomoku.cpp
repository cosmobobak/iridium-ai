#include "../games/Gomoku.hpp"

#include <array>
#include <tuple>

constexpr std::array test_bitmasks = {
    std::make_tuple(0, 0),
    std::make_tuple(0b11111111, 8),
    std::make_tuple(0b11111110, 7),
    std::make_tuple(0b11111101, 6),
    std::make_tuple(0b11111011, 5),
    std::make_tuple(0b11111111, 8),
    std::make_tuple(0b11111110, 7),
    std::make_tuple(0b11111101, 6),
    std::make_tuple(0b11111011, 5),
};

std::array test_bitstrings = {
    "0b00000000",
    "0b11111111",
    "0b11111110",
    "0b11111101",
    "0b11111011",
    "0b11111111",
    "0b11111110",
    "0b11111101",
    "0b11111011",
};

template < int N >
auto test_n_in_a_row(const std::tuple<int, int>& test) {
    auto [bb, count] = test;
    auto array = Gomoku::BitMatrix<15,15>::from_u64(bb);
    array.show();
    auto result = array.has_n_in_a_row(count);
    return result;
}

std::array results = {
    "FALSE",
    "TRUE",
};

int main(int argc, char const* argv[]) {
    int idx = 0;
    for (auto test : test_bitmasks) {
        auto result = test_n_in_a_row<5>(test);
        std::cout << "Test: "
                  << test_bitstrings[idx++] 
                  << " " 
                  << std::get<1>(test) 
                  << " " 
                  << results[result] 
                  << std::endl;
    }
    return 0;
}
