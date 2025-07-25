#pragma once

#include "Annotations.hpp"
#include "Concepts.hpp"

#include <algorithm>
#include <iterator>
#include <memory>
#include <utility>
#include <concepts>
#include <span>
#include <charconv>

namespace jser {

template<std::integral T, std::contiguous_iterator Iterator, std::sized_sentinel_for<Iterator> Sentinel>
requires (!std::same_as<T, bool>)
constexpr std::pair<T, Iterator> deserialize(Iterator begin, Sentinel end) {
    T rv;
    if (auto result = std::from_chars(std::to_address(begin), std::to_address(end), rv)) {
        return {rv, begin + (result.ptr - std::to_address(begin))};
    } else {
        throw std::system_error{std::make_error_code(result.ec), "Could not parse integer from characters"};
    }
}

template<std::same_as<std::string> T, typename Iterator, std::sentinel_for<Iterator> Sentinel>
constexpr std::pair<T, Iterator> deserialize(Iterator begin, Sentinel end) {
    if (begin == end) {
        throw std::runtime_error{"Empty data can't be a valid string"};
    }

    if (*begin != '"') {
        throw std::runtime_error{"Data does not start with quotes, can not be a string"};
    }

    ++begin;

    T string{};
    for (; begin != end; ++begin) {
        char c = *begin;

        if (c == '"') {
            return {std::move(string), begin + 1};
        } else if (c == '\\') {
            ++begin;
            if (begin == end) {
                throw std::runtime_error{"Data ended in escape sequence"};
            }

            char escaped = *begin;
            switch (escaped) {
                case '"':
                string.push_back('"');
                break;
            case '\\':
                string.push_back('\\');
                break;
            default:
                throw std::runtime_error{"Escaping is only implemented for quotes and backslash"};
            }
        } else {
            string.push_back(c);
        }
    }

    throw std::runtime_error{"Data ended before string end"};
}

}