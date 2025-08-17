#pragma once

#include <concepts>

namespace jser {

template<typename T, typename Iterator, std::sized_sentinel_for<Iterator> Sentinel>
constexpr std::pair<T, Iterator> deserialize(Iterator begin, Sentinel end);

constexpr auto serialize(const auto& value, auto out);

namespace readwrite::detail {

namespace concepts {

template<typename Candidate>
concept String = std::same_as<Candidate, std::string> || std::same_as<Candidate, std::string_view>;

}

constexpr auto skip_whitespace(auto pos, auto end) {
    while (pos != end && *pos == ' ') {
        ++pos;
    }
    return pos;
}

}

}