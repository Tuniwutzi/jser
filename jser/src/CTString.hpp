#pragma once

#include <array>
#include <algorithm>
#include <string_view>

namespace jscheme::detail {

template<size_t capacity>
struct CTString: std::array<char, capacity> {
    consteval CTString(const char(&literal)[capacity]) {
        if (literal[capacity-1] != 0) {
            throw std::runtime_error{"CTString must be 0-terminated"};
        }

        std::ranges::copy(literal, this->data());
    }

    constexpr std::string_view string_view() const noexcept {
        return {this->data()};
    }
};


}