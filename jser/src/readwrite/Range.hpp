#pragma once

#include "Helpers.hpp"

#include <ranges>
#include <stdexcept>

namespace jser::readwrite {

namespace detail::concepts {

template<typename Candidate>
concept PushBackRange = std::ranges::range<Candidate> && requires(Candidate c, std::ranges::range_value_t<Candidate> v) {
    c.push_back(std::move(v));
};

}

template<detail::concepts::PushBackRange Range>
requires(!detail::concepts::String<Range>)
constexpr auto read(auto begin, auto end) {
    if (end - begin < 2) {
        throw std::runtime_error{"Not enough characters to parse a json list"};
    }

    if (*begin != '[') {
        throw std::runtime_error{"Invalid starting character for json list"};
    }

    ++begin;
    begin = detail::skip_whitespace(begin, end);
    if (begin == end) {
        throw std::runtime_error{"Unexpected end of json list"};
    }
    
    using V = std::ranges::range_value_t<Range>;

    Range range{};
    if (*begin == ']') {
        return std::pair {
            std::move(range),
            begin + 1,
        };
    }

    while (true) {
        auto [value, pos] = ::jser::deserialize<V>(begin, end);
        range.push_back(std::move(value));
        begin = detail::skip_whitespace(std::move(pos), end);

        if (begin == end) {
            throw std::runtime_error{"Expected ']' to end json list"};
        }

        if (*begin == ',') {
            begin = detail::skip_whitespace(begin + 1, end);
            continue;
        } else {
            break;
        }
    }

    if (*begin != ']') {
        throw std::runtime_error{"Expected ']' to end json list"};
    }
    ++begin;

    return std::pair{
        range,
        begin,
    };
}

constexpr auto write(const std::ranges::input_range auto& range, auto out) {
    *out = '[';
    ++out;

    bool first = true;
    for (const auto& element: range) {
        if (first) {
            first = false;
        } else {
            *out = ',';
            ++out;
        }
        out = jser::serialize(element, out);
    }

    *out = ']';
    ++out;
    return out;
}

}