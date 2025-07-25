#pragma once

#include "DeserializeBasic.hpp"
#include "DeserializeCustomType.hpp"
#include "DeserializeObject.hpp"

namespace jser {

template<typename T, typename Range>
constexpr T deserialize(const Range& range) {
    auto [value, pos] = deserialize<T>(std::ranges::begin(range), std::ranges::end(range));
    if (pos != std::ranges::end(range)) {
        throw std::runtime_error{"Did not deserialize entire range"};
    }
    return std::move(value);
}

}