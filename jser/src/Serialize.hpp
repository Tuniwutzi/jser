#pragma once

#include "SerializeBasic.hpp"
#include "SerializeObject.hpp"

namespace jser {

constexpr std::string serialize(const auto& value) {
    std::string buf{};
    serialize(value, std::back_inserter(buf));
    return buf;
}

}