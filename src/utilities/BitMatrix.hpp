#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <numeric>
#include <vector>

#include "./bitset"

constexpr auto generate_bitmask(int n) -> uint64_t {
    if (n == 0) {
        return 0xffffffffffffffff;
    }
    // return a bitmask of width n
    return (1ULL << (n + 2)) - 1;
}

template <int WIDTH, int HEIGHT>
constexpr auto columnar_bitmask() -> std::bitset<WIDTH * HEIGHT> {
    std::bitset<WIDTH * HEIGHT> out;
    for (int i = 0; i < HEIGHT; i++) {
        out.set(i * WIDTH);
    }
    return out;
}

template <int WIDTH, int HEIGHT>
class BitMatrix {
   public:
    // A bit matrix of size WIDTH x HEIGHT.
    static constexpr auto SIZE = WIDTH * HEIGHT;
    using bitvec = std::bitset<SIZE>;
    static constexpr bitvec COL_MASK = columnar_bitmask<WIDTH, HEIGHT>();
    bitvec data;

    // an enum of directions for checking rows
    enum Direction {
        VERTICAL,
        HORIZONTAL,
        DIAGONAL_45,
        DIAGONAL_135,
    };

   public:
    static auto from_u64(uint64_t num) noexcept -> BitMatrix {
        BitMatrix result;
        result.data = num;
        return result;
    }

    auto operator[](int x) const noexcept -> uint64_t {
        return data[x];
    }

    auto operator()(int x, int y) const noexcept -> uint64_t {
        // access a bit at position x, y
        auto linear_index = x + y * WIDTH;
        return data[linear_index];
    }

    auto popcount() const noexcept -> int {
        return data.count();
    }

    auto ctz() const noexcept -> int {
        // horrible terrible slow approach
        for (int i = 0; i < SIZE; i++) {
            if (data[i]) {
                return i - 1;
            }
        }
        assert(false);
        return SIZE * 64;
    }

    void reset() {
        // zero out the matrix
        std::fill(data.begin(), data.end(), 0ULL);
    }

    auto test_bit(int x) const noexcept -> bool {
        // test if a bit at position x is set
        return data.test(x);
    }

    auto test_bit(int x, int y) const noexcept -> bool {
        // test if a bit at position x, y is set
        auto linear_index = x + y * WIDTH;
        return data.test(linear_index);
    }

    void set_bit(int x) {
        // set a bit at position x
        data.set(x);
    }

    void clear_bit(int x) {
        // clear a bit at position x
        data.reset(x);
    }

    void flip() {
        // flip all bits
        data.flip();
    }

    void bitscan(std::vector<uint_fast8_t>& positions) const {
        assert(positions.empty());
        positions.reserve(popcount());
        // find all the bits set in the matrix
        for (int i = 0; i < SIZE; ++i) {
            if (data[i]) {
                positions.push_back(i);
            }
        }
    }

    auto nth_bit(int n) const -> int {
        // return the index of the nth bit set
        // go through the bits set in the matrix
        while (n) {
            if (data[n]) {
                --n;
            }
        }
        return n;
    }

    void clear_bit() {
        // this function shouldn't really be needed
        assert(false);
    }

    enum class BShiftDir { LEFT, RIGHT, };
    template <BShiftDir Dir>
    static constexpr auto safety_mask(int width) -> std::bitset<SIZE> {
        // a bitmask of all the bits that are safe to set
        std::bitset<SIZE> out;
        if constexpr (Dir == BShiftDir::LEFT) {
            for (int i = 0; i < width; i++) {
                out |= COL_MASK << (i * HEIGHT);
            }
        } else {
            for (int i = 0; i < width; i++) {
                out |= COL_MASK >> (i * HEIGHT);
            }
        }
        return out;
    }

    static constexpr auto safe_bitshiftl(bitvec bb, int n) -> bitvec {
        // when we shift the data, we have to ensure that we aren't shifting across edges.
        // because shifts operate forward and backward, the edges we will run over are the left and right edges.
        // for this reason, we want a vertical bitmask "column"
        // the column is marked by on-bits at the start of each row
        auto mask = safety_mask<BShiftDir::LEFT>(n);
        show_bitvec(mask);
        puts("");
        bb <<= n;
        bb &= mask;
        return bb;
    }

    static constexpr auto safe_bitshiftr(bitvec bb, int n) -> bitvec {
        // when we shift the data, we have to ensure that we aren't shifting across edges.
        // because shifts operate forward and backward, the edges we will run over are the left and right edges.
        // for this reason, we want a vertical bitmask "column"
        // the column is marked by on-bits at the start of each row
        auto mask = safety_mask<BShiftDir::RIGHT>(n);
        show_bitvec(mask);
        puts("");
        bb >>= n;
        bb &= mask;
        return bb;
    }

    auto operator<<(int n) -> BitMatrix {
        // shift the data
        data = safe_bitshiftl(data, n);
        return *this;
    }

    auto operator>>(int n) -> BitMatrix {
        // shift the data
        data = safe_bitshiftr(data, n);
        return *this;
    }

    template <int N, Direction dir>
    auto has_n_in_a_row() const -> bool {
        std::array<int, N> bitshifts;
        if constexpr (dir == Direction::HORIZONTAL) {
            // simple special case for horizontal
            std::iota(bitshifts.begin(), bitshifts.end(), 0);
        } else if (dir == Direction::VERTICAL) {
            // increase by a bitshift that moves everything up
            for (int i = 0; i < N; ++i) {
                bitshifts[i] = i * WIDTH;
            }
        } else if (dir == Direction::DIAGONAL_45) {
            // increase by a bitshift that moves everything up and to the right
            for (int i = 0; i < N; ++i) {
                bitshifts[i] = i * WIDTH + i;
            }
        } else if (dir == Direction::DIAGONAL_135) {
            // increase by a bitshift that moves everything up and to the left
            for (int i = 0; i < N; ++i) {
                bitshifts[i] = i * WIDTH - i;
            }
        }

        auto result = data;
        for (auto shift : bitshifts) {
            result &= safe_bitshiftl(data, shift);
        }

        return result.any();
    }

    template <Direction dir>
    auto has_n_in_a_row(int n) const -> bool {
        std::vector<int> bitshifts(n);
        if constexpr (dir == Direction::HORIZONTAL) {
            // simple special case for horizontal
            std::iota(bitshifts.begin(), bitshifts.end(), 0);
        } else if (dir == Direction::VERTICAL) {
            // increase by a bitshift that moves everything up
            for (int i = 0; i < n; ++i) {
                bitshifts[i] = i * WIDTH;
            }
        } else if (dir == Direction::DIAGONAL_45) {
            // increase by a bitshift that moves everything up and to the right
            for (int i = 0; i < n; ++i) {
                bitshifts[i] = i * WIDTH + i;
            }
        } else if (dir == Direction::DIAGONAL_135) {
            // increase by a bitshift that moves everything up and to the left
            for (int i = 0; i < n; ++i) {
                bitshifts[i] = i * WIDTH - i;
            }
        }

        auto result = data;
        for (auto shift : bitshifts) {
            result &= safe_bitshiftl(data, shift);
        }

        return result.any();
    }

    template <int N>
    auto has_n_in_a_row() const -> bool {
        using enum Direction;
        // printf("has_n_in_a_row(%d, %d, %d)\n", x, y, n);
        // check if there are n in a row in any direction
        return has_n_in_a_row<N, VERTICAL>() ||
               has_n_in_a_row<N, HORIZONTAL>() ||
               has_n_in_a_row<N, DIAGONAL_45>() ||
               has_n_in_a_row<N, DIAGONAL_135>();
    }

    auto has_n_in_a_row(int n) const -> bool {
        using enum Direction;
        // printf("has_n_in_a_row(%d, %d, %d)\n", x, y, n);
        // check if there are n in a row in any direction
        return has_n_in_a_row<VERTICAL>(n) ||
               has_n_in_a_row<HORIZONTAL>(n) ||
               has_n_in_a_row<DIAGONAL_45>(n) ||
               has_n_in_a_row<DIAGONAL_135>(n);
    }

    friend auto operator|(const BitMatrix& lhs, const BitMatrix& rhs) -> BitMatrix {
        // take the union of two bit matrices
        auto result = BitMatrix<WIDTH, HEIGHT>();
        result.data = lhs.data | rhs.data;
        return result;
    }

    friend auto operator==(const BitMatrix& lhs, const BitMatrix& rhs) -> bool {
        // test if two bit matrices are equal
        return lhs.data == rhs.data;
    }

    // conversion to bool
    operator bool() const {
        return data.any();
    }

    void show() const {
        for (int y = 0; y < HEIGHT; ++y) {
            for (int x = 0; x < WIDTH; ++x) {
                if (test_bit(x, y)) {
                    printf("1");
                } else {
                    printf("0");
                }
            }
            printf("\n");
        }
    }

    static void show_bitvec(const bitvec bb) {
        for (int y = 0; y < HEIGHT; ++y) {
            for (int x = 0; x < WIDTH; ++x) {
                int i = y * WIDTH + x;
                if (bb.test(i)) {
                    printf("1");
                } else {
                    printf("0");
                }
            }
            printf("\n");
        }
    }
};