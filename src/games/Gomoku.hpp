#pragma once

#include "../utilities/bitset"
#include "../utilities/random.hpp"
#include <array>
#include <iostream>
#include <vector>
#include <cassert>
#include <numeric>
#include <algorithm>

namespace Gomoku {

template <typename T>
[[nodiscard]] auto zipstring(std::vector<T> v1, std::vector<T> v2) -> std::string {
    assert(v1.size() == v2.size());
    std::string builder;
    builder.append("{ ");
    for (size_t i = 0; i < v1.size(); i++) {
        // builder.append((std::to_string)(i));
        builder += '(';
        builder.append(std::to_string(v1[i]));
        builder += ',';
        builder.append(std::to_string(v2[i]));
        builder += ')';
        builder.append(", ");
    }
    builder.append("}");
    return builder;
}

[[nodiscard]] constexpr auto generate_bitmask(int n) -> uint64_t {
    if (n == 0) {
        return 0xffffffffffffffff;
    }
    // return a bitmask of width n
    return (1ULL << (n + 2)) - 1;
}

template <int WIDTH, int HEIGHT>
[[nodiscard]] constexpr auto columnar_bitmask() -> std::bitset<WIDTH * HEIGHT> {
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
    [[nodiscard]] static auto from_u64(uint64_t num) noexcept -> BitMatrix {
        BitMatrix result;
        result.data = num;
        return result;
    }

    [[nodiscard]] auto operator[](int x) const noexcept -> uint64_t {
        return data[x];
    }

    [[nodiscard]] auto operator()(int x, int y) const noexcept -> uint64_t {
        // access a bit at position x, y
        auto linear_index = x + y * WIDTH;
        return data[linear_index];
    }

    [[nodiscard]] auto popcount() const noexcept -> int {
        return data.count();
    }

    [[nodiscard]] auto ctz() const noexcept -> int {
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

    [[nodiscard]] auto test_bit(int x) const noexcept -> bool {
        // test if a bit at position x is set
        return data.test(x);
    }

    [[nodiscard]] auto test_bit(int x, int y) const noexcept -> bool {
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
        positions.resize(popcount());
        int idx = 0;
        // find all the bits set in the matrix
        for (int i = 0; i < SIZE; ++i) {
            if (data[i]) {
                positions[idx++] = i;
            }
        }
    }

    void clear_bit() {
        // this function shouldn't really be needed
        assert(false);
    }

    [[nodiscard]] static constexpr auto safe_bitshift(bitvec bb, int n) -> bitvec {
        // when we shift the data, we have to ensure that we aren't shifting across edges.
        // because shifts operate forward and backward, the edges we will run over are the left and right edges.
        // for this reason, we want a vertical bitmask "column"
        bitvec mask = 0;
        for (int i = 0; i < n; i++) {
            mask |= COL_MASK << i;
        }
        bb << n;
        bb &= mask;
        return bb;
    }

    template < int N , Direction dir >
    auto has_n_in_a_row() const -> bool {
        std::array < int, N > bitshifts;
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
            result &= safe_bitshift(data, shift);
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
            result &= safe_bitshift(data, shift);
        }

        return result.any();
    }

    template <int N>
    auto has_n_in_a_row() const -> bool {
        // printf("has_n_in_a_row(%d, %d, %d)\n", x, y, n);
        // check if there are n in a row in any direction
        return has_n_in_a_row<N, Direction::VERTICAL>() ||
               has_n_in_a_row<N, Direction::HORIZONTAL>() ||
               has_n_in_a_row<N, Direction::DIAGONAL_45>() ||
               has_n_in_a_row<N, Direction::DIAGONAL_135>();
    }

    auto has_n_in_a_row(int n) const -> bool {
        // printf("has_n_in_a_row(%d, %d, %d)\n", x, y, n);
        // check if there are n in a row in any direction
        return has_n_in_a_row<Direction::VERTICAL>(n) ||
               has_n_in_a_row<Direction::HORIZONTAL>(n) ||
               has_n_in_a_row<Direction::DIAGONAL_45>(n) ||
               has_n_in_a_row<Direction::DIAGONAL_135>(n);
    }

    friend auto operator|(const BitMatrix& lhs, const BitMatrix& rhs) -> BitMatrix {
        // take the union of two bit matrices
        auto result = BitMatrix<WIDTH, HEIGHT>();
        for (int i = 0; i < SIZE; ++i) {
            result.data[i] = lhs.data[i] | rhs.data[i];
        }
        return result;
    }

    friend auto operator==(const BitMatrix& lhs, const BitMatrix& rhs) -> bool {
        // test if two bit matrices are equal
        for (int i = 0; i < SIZE; ++i) {
            if (lhs.data[i] != rhs.data[i]) {
                return false;
            }
        }
        return true;
    }

    void show() const {
        for (int x = 0; x < WIDTH; ++x) {
            for (int y = 0; y < HEIGHT; ++y) {
                if (test_bit(x, y)) {
                    printf("1");
                } else {
                    printf("0");
                }
            }
            printf("\n");
        }
    }
};

template < int WIDTH = 8, int HEIGHT = 8 >
class State {
   public:
    using Move = uint_fast8_t;
    using BB = BitMatrix<WIDTH, HEIGHT>;
    static constexpr auto GAME_SOLVABLE = false;
    static constexpr auto GAME_EXP_FACTOR = 8;
    static constexpr auto MAX_GAME_LENGTH = WIDTH * HEIGHT;
    static constexpr auto NUM_ACTIONS = WIDTH * HEIGHT;
    static constexpr std::array<char, 2> players = {'X', 'O'};

   private:
    std::array<BB, 2> node;
    int move_count;
    // std::array<Move, MAX_GAME_LENGTH> move_stack = {0};

   public:
    State() {
        move_count = 0;
    }

    [[nodiscard]] auto get_turn() const -> int {
        return (move_count & 1) ? -1 : 1;
    }

    [[nodiscard]] auto get_move_count() const -> int {
        return move_count;
    }

    [[nodiscard]] auto get_turn_index() const -> int {
        return move_count & 1;
    }

    [[nodiscard]] auto is_full() const -> bool {
        return move_count == MAX_GAME_LENGTH;
    }
    
    void reset() {
        node[0].reset();
        node[1].reset();
        // std::fill(move_stack.begin(), move_stack.end(), 0);
        move_count = 0;
    }

    void show() const {
        int row, col;
        for (row = 0; row < HEIGHT; ++row) {
            for (col = 0; col < WIDTH; ++col) {
                if (pos_filled(row * WIDTH + col)) {
                    if (player_at(row * WIDTH + col))
                        std::cout << "X ";
                    else
                        std::cout << "0 ";
                } else
                    std::cout << ". ";
            }
            std::cout << '\n';
        }
        std::cout << '\n';
    }

    void mem_setup() {
        // move_stack.reserve(MAX_GAME_LENGTH);
    }

    void set_move_count(int n) {
        move_count = n;
    }

    [[nodiscard]] auto get_node() const -> const std::array<BB, 2>& {
        return node;
    }

    [[nodiscard]] auto player_at(int i) const -> bool {
        //only valid to use if pos_filled() returns true, true = x, false = y
        return node[0].test_bit(i);
    }

    [[nodiscard]] auto player_at(int row, int col) const -> bool {
        return node[0].test_bit(row * WIDTH + col);
    }

    [[nodiscard]] auto pos_filled(int i) const -> bool {
        return (node[0] | node[1]).test_bit(i);
    }

    [[nodiscard]] auto pos_filled(int row, int col) const -> bool {
        return (node[0] | node[1]).test_bit(row * WIDTH + col);
    }

    [[nodiscard]] auto probe_spot(int i) const -> bool {
        // tests the bit of the most recently played side
        return node[(move_count + 1) & 1].test_bit(i);
    }

    [[nodiscard]] auto probe_spot(int row, int col) const -> bool {
        // tests the bit of the most recently played side
        return node[(move_count + 1) & 1].test_bit(row * WIDTH + col);
    }

    [[nodiscard]] auto num_legal_moves() const -> size_t {
        return WIDTH * HEIGHT - (node[0] | node[1]).popcount();
    }

    [[nodiscard]] auto legal_moves() const -> std::vector<Move> {
        std::vector<Move> moves;

        auto bb = node[0] | node[1];

        // put bits in all the empty slots to play
        bb.flip();

        // this inserts all the empty slots into the moves vector
        bb.bitscan(moves);
        
        return moves;
    }

    void random_play() {
        auto bb = node[0] | node[1];
        bb.flip();
        auto option_count = bb.popcount();
        auto idx = random_int(option_count);
        for (int i = 0; i < idx; ++i) {
            bb.clear_bit();
        }

        auto move = bb.ctz();

        play(move);
    }

    void pass_turn() {
        move_count++;
    }

    void unpass_turn() {
        move_count--;
    }

    void play(int i) {
        // move_count acts to determine which colour is played
        node[move_count & 1].set_bit(i);
        // store the made move in the stack
        // move_stack[move_count] = i;
        move_count++;
    }

    void unplay() {
        // decrement move counter and get the most recently played move
        // -------------- int i = move_stack[--move_count];
        // a bit is removed by XOR
        // -------------- node[move_count & 1] ^= (1ULL << i);
    }

    void unplay(int i) {
        // decrement move counter and get the most recently played move
        --move_count;
        // a bit is removed
        node[move_count & 1].clear_bit(i);
    }

    [[nodiscard]] auto is_game_over() const -> bool {
        return is_full() || evaluate();
    }
   private:
    [[nodiscard]] auto horizontal_term() const -> int {
        // iterates the starting positions of the rows
        for (Move row = 0; row < WIDTH * HEIGHT; row += WIDTH) {
            for (Move i = row; i < row + WIDTH - 4; i++) {
                if (probe_spot(i) &&
                    probe_spot(i + 1) &&
                    probe_spot(i + 2) &&
                    probe_spot(i + 3) &&
                    probe_spot(i + 4)) {
                    return -get_turn();
                }
            }
        }
        return 0;
    }

    [[nodiscard]] auto vertical_term() const -> int {
        // iterates the starting positions of the columns
        for (Move col = 0; col < WIDTH; col++) {
            // this line below could be fucky
            for (Move i = col; i < col + (WIDTH * (HEIGHT - 4)); i += WIDTH) {
                if (probe_spot(i) &&
                    probe_spot(i + WIDTH * 1) &&
                    probe_spot(i + WIDTH * 2) &&
                    probe_spot(i + WIDTH * 3) &&
                    probe_spot(i + WIDTH * 4)) {
                    return -get_turn();
                }
            }
        }
        return 0;
    }

    [[nodiscard]] auto diagdown_term() const -> int {
        // iterates the starting positions of the rows
        for (Move row = 0; row < HEIGHT - 4; row++) {
            for (Move col = 0; col < WIDTH - 4; col++) {
                if (probe_spot(row, col) &&
                    probe_spot(row + 1, col + 1) &&
                    probe_spot(row + 2, col + 2) &&
                    probe_spot(row + 3, col + 3) &&
                    probe_spot(row + 4, col + 4)) {
                    return -get_turn();
                }
            }
        }
        return 0;
    }

    [[nodiscard]] auto diagup_term() const -> int {
        // iterates the starting positions of the rows
        for (Move row = 4; row < HEIGHT; row++) {
            for (Move col = 0; col < WIDTH - 4; col++) {
                if (probe_spot(row, col) &&
                    probe_spot(row - 1, col + 1) &&
                    probe_spot(row - 2, col + 2) &&
                    probe_spot(row - 3, col + 3) &&
                    probe_spot(row - 4, col + 4)) {
                    return -get_turn();
                }
            }
        }
        return 0;
    }

    [[nodiscard]] auto nevaluate() const -> int {
        auto eval = 0;
        if (node[0].has_n_in_a_row(5)){
            eval = 1;
        } else if (node[1].has_n_in_a_row(5)) {
            eval = -1;
        }
        return eval;
    }

   public:
    [[nodiscard]] auto evaluate() const -> int {
        if (!move_count)
            return 0;

        auto out = 0;
        int v, h, u, d;
        v = vertical_term();
        if (v) {
            out = v;
            goto ret;
        }
        h = horizontal_term();
        if (h) {
            out = h;
            goto ret;
        }
        u = diagup_term();
        if (u) {
            out = u;
            goto ret;
        }
        d = diagdown_term();
        if (d) {
            out = d;
            goto ret;
        }

       ret:
        out = 0;

        auto new_eval = nevaluate();

        printf("new eval: %d\n", new_eval);
        printf("old eval: %d\n", out);

        return out;
    }

    void show_result() {
        int r;
        r = evaluate();
        if (r == 0) {
            std::cout << "1/2-1/2" << '\n';
        } else if (r > 0) {
            std::cout << "1-0" << '\n';
        } else {
            std::cout << "0-1" << '\n';
        }
    }

    [[nodiscard]] auto get_player_move() const -> Move {
        const std::vector<Move> legals = legal_moves();
        std::vector<Move> shiftedLegals;
        std::transform(legals.begin(), legals.end(), std::back_inserter(shiftedLegals), [](Move n) { return n + 1; });
        std::vector<Move> rows, cols;
        for (auto m : legals) {
            rows.push_back(m / WIDTH + 1);
            cols.push_back(m % WIDTH + 1);
        }

        std::cout << "Your legal moves are: " << zipstring(rows, cols) << "\n";
        int row, col, pos;
        std::cout << "Enter row: ";
        std::cin >> row;
        std::cout << "Enter col: ";
        std::cin >> col;
        pos = (row - 1) * WIDTH + (col - 1);
        while (std::none_of(legals.begin(), legals.end(), [pos](Move m) { return m == (pos); })) {
            std::cout << "invalid move.\n";
            std::cout << "Enter row: ";
            std::cin >> row;
            std::cout << "Enter col: ";
            std::cin >> col;
            pos = (row - 1) * WIDTH + (col - 1);
        }
        return pos;
    }

    [[nodiscard]] auto heuristic_value() const -> uint_fast8_t {
        return 0;
    }

    friend auto operator==(const State& a, const State& b) -> bool {
        return a.node == b.node;
    }
};

}  // namespace Gomoku