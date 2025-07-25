#pragma once

#include "FormatterConcepts.hpp"

#include <array>
#include <charconv>
#include <concepts>
#include <format>

namespace jser::formatters {

namespace detail {

template<typename Iterator, bool first, typename Parent>
class BasicObjectFormatter;
template<typename Iterator, bool first, typename Parent>
class BasicArrayFormatter;

template<typename Iterator, typename Parent>
class BasicFormatter {
public:
    constexpr BasicFormatter(Iterator out)
    :_out(std::move(out))
    {}

    template<concepts::Number T>
    constexpr Parent format_value(T value) {
        if consteval {
            std::array<char, 512> buffer;
            auto result = std::to_chars(std::to_address(buffer.begin()), std::to_address(buffer.end()), value, 10);
            if (result) {
                _out = std::copy(buffer.data(), result.ptr, _out);
            } else {
                throw std::runtime_error{"Could not stringify value at compile time"};
            }
        } else {
            _out = std::format_to(_out, "{}", value);
        }

        return {std::move(_out)};
    }

    template<concepts::Stringlike T>
    constexpr Parent format_value(const T& value) {
        *_out = '"';
        ++_out;
        _out = std::ranges::copy(value, _out).out;
        *_out = '"';
        ++_out;
        return {std::move(_out)};
    }

    constexpr Parent format_value(bool value) {
        _out = std::ranges::copy(value ? "true" : "false", _out).out;
        return {std::move(_out)};
    }

    constexpr BasicObjectFormatter<Iterator, true, BasicFormatter> format_object() {
        return {std::move(_out)};
    }

    constexpr BasicArrayFormatter<Iterator, true, BasicFormatter> format_array() {
        return {std::move(_out)};
    }

private:
    Iterator _out;
};

template<typename Iterator, bool first, typename Parent>
class BasicObjectFormatter {
public:
    constexpr BasicObjectFormatter(Iterator out)
    :_out{std::move(out)}
    {
        _out = '{';
        ++_out;
    }

    BasicFormatter<Iterator, BasicObjectFormatter<Iterator, false, Parent>> add_field(std::string_view name) {
        if constexpr (!first) {
            *_out = ',';
            ++_out;
        }

        BasicFormatter<Iterator, Iterator> name_formatter{_out};
        _out = name_formatter.format_value(name);

        return { std::move(_out) };
    }

    Parent finalize() {
        *_out = '}';
        ++_out;
        return {std::move(_out)};
    }

private:
    Iterator _out;
};

template<typename Iterator, bool first, typename Parent>
class BasicArrayFormatter {
public:
    constexpr BasicArrayFormatter(Iterator out)
    :_out{std::move(out)}
    {
        _out = '[';
        ++_out;
    }

    BasicFormatter<Iterator, BasicObjectFormatter<Iterator, false, Parent>> add() {
        if constexpr (!first) {
            *_out = ',';
            ++_out;
        }

        return { std::move(_out) };
    }

    Parent finalize() {
        *_out = ']';
        ++_out;
        return {std::move(_out)};
    }

private:
    Iterator _out;
};

static_assert(concepts::ObjectFormatter<BasicObjectFormatter<char*, true, char*>>);

static_assert(concepts::ArrayFormatter<BasicArrayFormatter<char*, true, char*>>);

static_assert(concepts::Formatter<BasicFormatter<char*, char*>>);

}

}