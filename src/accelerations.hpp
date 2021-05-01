#pragma once

#include <array>
#include <bitset>
#include <cassert>
#include <cstddef>
#include <functional>
#include <iostream>
#include <string>

template <typename T>
auto pop(std::vector<T> v) -> T {
    T result = v.back();
    v.pop_back();
    return result;
}

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

inline void copy_int8_array_81(const uint_fast8_t in[], uint_fast8_t out[]) {
    for (size_t i = 0; i < 81; i++) {
        out[i] = in[i];
    }
}

template <typename T>
auto zipstring(std::vector<T> v1, std::vector<T> v2) -> std::string {
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

auto zipstring(std::vector<short unsigned int> v1, std::vector<short unsigned int> v2) -> std::string {
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

auto zipstring(std::vector<short> v1, std::vector<short> v2) -> std::string {
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

auto string(std::vector<uint_fast8_t> v) -> std::string {
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

template <typename T>
void showvec(std::vector<T> v) {
    std::cout << "{ ";
    for (auto&& i : v) {
        std::cout << (int)i;
        std::cout << ", ";
    }
    std::cout << "}";
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