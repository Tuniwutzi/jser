#pragma once

#include "Traits.hpp"

#include <concepts>
#include <iterator>
#include <stdexcept>
#include <string_view>
#include <charconv>
#include <system_error>

namespace jser {

template<>
struct Traits<bool> {
    template<typename Iterator, std::sized_sentinel_for<Iterator> Sentinel>
    static constexpr std::pair<bool, Iterator> read(Iterator begin, Sentinel end) {
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


        return {result, begin};
    }
};

template<std::integral T>
requires(!std::same_as<T, bool>)
struct Traits<T> {
    template<typename I, typename S>
    static constexpr std::pair<T, I> read(I begin, S end) {
        T rv;
        if (auto result = std::from_chars(std::to_address(begin), std::to_address(end), rv)) {
            return {rv, begin + (result.ptr - std::to_address(begin))};
        } else {
            throw std::system_error{std::make_error_code(result.ec), "Could not parse integer from characters"};
        }
    }
};

template<>
struct Traits<std::string> {
    static constexpr auto read(auto begin, auto end) {
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
};

}
