#pragma once

#include "readwrite/Basic.hpp"

#include <iterator>
#include <span>

namespace jser {

template<typename T, typename Iterator, std::sized_sentinel_for<Iterator> Sentinel>
constexpr std::pair<T, Iterator> deserialize(Iterator begin, Sentinel end) {
    return readwrite::read<T>(begin, end);
}


template<typename T, typename Range>
constexpr T deserialize(const Range& range) {
    auto [value, pos] = deserialize<T>(std::ranges::begin(range), std::ranges::end(range));
    if (pos != std::ranges::end(range)) {
        throw std::runtime_error{"Did not deserialize entire range"};
    }
    return std::move(value);
}

template<typename T, size_t N>
constexpr T deserialize(const char(&literal)[N]) {
    auto end = literal[N-1] == 0 ? (literal + N - 1) : literal + N;
    return deserialize<T>(std::span { literal, end });
}

}