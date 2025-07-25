#pragma once

#include "Annotations.hpp"
#include "detail/Helpers.hpp"
#include "Concepts.hpp"

#include <algorithm>
#include <concepts>
#include <format>
#include <array>
#include <charconv>
#include <iterator>
#include <meta>
#include <string_view>

namespace jser {

namespace detail {

template<typename Serializer, typename T, typename OutIt>
constexpr OutIt serialize_custom_type(const T& value, OutIt iterator);

}

template<std::integral T, typename OutIt>
requires (!std::same_as<T, bool>)
constexpr OutIt serialize(T value, OutIt iterator) {
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

template<concepts::String T, typename OutIt>
constexpr OutIt serialize(const T& value, OutIt iterator) {
    detail::write('\"', iterator);
    iterator = std::ranges::copy(value, iterator).out;
    detail::write('\"', iterator);
    return iterator;
}

template<concepts::CustomType T, typename OutIt>
constexpr OutIt serialize(const T& value, OutIt iterator) {
    using namespace detail;
    constexpr auto annotation = get_unique_annotation(^^T, ^^CustomType);
    constexpr auto template_arg = template_arguments_of(type_of(annotation)).front();
    if constexpr (template_arg == ^^void) {
        return serialize_custom_type<T>(value, iterator);
    } else {
        using Serializer = [:template_arg:];
        return serialize_custom_type<Serializer>(value, iterator);
    }
}

namespace detail {

template<typename Serializer, typename T, typename OutIt>
constexpr OutIt serialize_custom_type(const T& value, OutIt iterator) {
    static constexpr auto serialize_function = [] consteval {
        auto serializers = std::ranges::to<std::vector>(members_of(^^Serializer, std::meta::access_context::current()) | std::views::filter([](auto member) consteval {
            if (is_special_member_function(member)) {
                return false;
            }
            
            if (!is_static_member(member)) {
                return false;
            }
            
            if (identifier_of(member) != "serialize") {
                return false;
            }

            return true;
        }));

        if (serializers.size() != 1) {
            throw CTException{"None or too many members 'serialize' found in serializer"};
        }

        if (parameters_of(serializers.front()).size() != 1) {
            // TODO: this is for performance optimization
            // Future custom serializers can choose to expect an object/array/string context
            // that they can directly add values into to avoid having to create and return some
            // intermediate representation
            throw CTException{"Special serialization not yet implemented"};
        }

        return serializers.front();
    }();

    auto function = [:serialize_function:];
    return serialize(function(value), iterator);
}

}

}