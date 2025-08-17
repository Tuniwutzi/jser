#pragma once

#include "readwrite/Basic.hpp"
#include "readwrite/Range.hpp"

#include <string>

namespace jser {

constexpr auto serialize(const auto& value, auto out) {
    return readwrite::write(value, out);
}

constexpr std::string serialize(const auto& value) {
    std::string buf{};
    serialize(value, std::back_inserter(buf));
    return buf;
}

}