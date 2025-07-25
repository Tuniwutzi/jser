#pragma once

#include <string_view>

namespace jser::detail {

template<typename T>
struct ObjectInfo {
    static constexpr auto fields = std::define_static_array(nonstatic_data_members_of(^^T, std::meta::access_context::unchecked()));
};

constexpr void write(char c, auto& iterator) {
    *iterator = c;
    ++iterator;
}

constexpr void write(std::string_view sv, auto& iterator) {
    iterator = std::ranges::copy(sv, iterator).out;
}

}