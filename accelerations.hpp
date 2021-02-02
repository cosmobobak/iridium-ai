#pragma once

#include <array>
#include <bitset>
#include <cstddef>
#include <functional>
#include <iostream>
#include <string>

template <typename T, std::size_t N>
struct std::hash<std::array<T, N> > {
    std::size_t operator()(const std::array<T, N>& arr) const {
        /* 32 bit FNV-1a hash */
        std::size_t h = 2166136261;
        for (const auto& item : arr) {
            h ^= std::hash<T>()(item);
            h *= 16777619;
        }
        return h;
    }
};

inline auto popcount(const unsigned int bb) -> int_fast8_t {
    return __builtin_popcount(bb);
}

auto zipstring(std::vector<int> v1, std::vector<int> v2) -> std::string {
    assert(v1.size() == v2.size());
    std::string builder;
    builder.append("{ ");
    for (int i = 0; i < v1.size(); i++) {
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

auto zipstring(std::vector<short> v1, std::vector<short> v2) -> std::string {
    assert(v1.size() == v2.size());
    std::string builder;
    builder.append("{ ");
    for (int i = 0; i < v1.size(); i++) {
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

auto string(std::vector<int_fast8_t> v) -> std::string {
    std::string builder;
    builder.append("{ ");
    for (auto&& i : v) {
        // builder.append((std::to_string)(i));
        builder.append(std::to_string(i));
        builder.append(", ");
    }
    builder.append("}");
    return builder;
}

template <class T>
auto string(std::vector<T> v) -> std::string {
    std::string builder;
    builder.append("{ ");
    for (auto&& i : v) {
        // builder.append((std::to_string)(i));
        builder += (char)(i);
        builder.append(", ");
    }
    builder.append("}");
    return builder;
}

inline auto lsb(int_fast16_t bitboard) -> int_fast8_t {
    return __builtin_ffs(bitboard) - 1;
}