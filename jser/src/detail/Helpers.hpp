#pragma once

#include <string_view>

namespace jscheme::detail {

constexpr void write(char c, auto& iterator) {
    *iterator = c;
    ++iterator;
}

constexpr void write(std::string_view sv, auto& iterator) {
    iterator = std::ranges::copy(sv, iterator).out;
}

}