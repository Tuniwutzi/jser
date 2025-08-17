#pragma once

#include "Helpers.hpp"

#include <array>
#include <concepts>
#include <iterator>
#include <stdexcept>
#include <string_view>
#include <charconv>
#include <system_error>

namespace jser::readwrite {

template<std::same_as<bool>>
constexpr auto read(auto begin, auto end) {
    if (begin == end) {
        throw std::runtime_error{"Too few bytes to deserialize a boolean"};
    }

    bool result;
    std::string_view expectation;
    switch (*begin) {
    case 't':
        result = true;
        expectation = "rue";
        break;
    case 'f':
        result = false;
        expectation = "alse";
        break;
    default:
        throw std::runtime_error{"Invalid character found in boolean"};
    }
    ++begin;

    for (auto c: expectation) {
        if (begin == end) {
            throw std::runtime_error{"Too few bytes to deserialize a boolean"};
        }

        if (*begin != c) {
            throw std::runtime_error{std::format("Unexpected character {} in boolean", *begin)};
        }

        ++begin;
    }

    return std::pair{result, begin};
}

constexpr auto write(bool value, auto out) {
    if (value) {
        return std::ranges::copy(std::string_view{"true"}, out).out;
    } else {
        return std::ranges::copy(std::string_view{"false"}, out).out;
    }
}

    
template<std::integral T>
requires(!std::same_as<T, bool>)
constexpr auto read(auto begin, auto end) {
    T rv;
    if (auto result = std::from_chars(std::to_address(begin), std::to_address(end), rv)) {
        return std::pair{rv, begin + (result.ptr - std::to_address(begin))};
    } else {
        throw std::system_error{std::make_error_code(result.ec), "Could not parse integer from characters"};
    }
}

template<std::integral T>
requires (!std::same_as<T, bool>)
constexpr auto write(T value, auto iterator) {
    if consteval {
        std::array<char, 512> buffer;
        auto result = std::to_chars(std::to_address(buffer.begin()), std::to_address(buffer.end()), value, 10);
        if (result) {
            return std::copy(buffer.data(), result.ptr, iterator);
        } else {
            throw std::runtime_error{"Could not stringify value at compile time"};
        }
    } else {
        return std::format_to(iterator, "{}", value);
    }
}

// Only std::string for now
// But with contiguous iterators, we could deserialize into a string_view referencing the data in place
template<std::same_as<std::string>>
constexpr auto read(auto begin, auto end) {
    if (begin == end) {
        throw std::runtime_error{"Empty data can't be a valid string"};
    }

    if (*begin != '"') {
        throw std::runtime_error{"Data does not start with quotes, can not be a string"};
    }

    ++begin;

    std::string string{};
    for (; begin != end; ++begin) {
        char c = *begin;

        if (c == '"') {
            return std::pair{std::move(string), begin + 1};
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

template<detail::concepts::String S>
constexpr auto write(const S& value, auto out) {
    *out = '"';
    ++out;
    out = std::ranges::copy(value, out).out;
    *out = '"';
    ++out;
    return out;
}

template<size_t N>
constexpr auto write(const char(&value)[N], auto out) {
    if (value[N-1] != 0) {
        return write(std::string_view{value, N-1}, out);
    } else {
        return write(std::string_view{value}, out);
    }
}

}
