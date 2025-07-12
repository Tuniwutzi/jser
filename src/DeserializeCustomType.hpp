#pragma once

#include "Concepts.hpp"
#include <iterator>

namespace jscheme {

namespace detail {

template<typename Serializer, typename Iterator, std::sentinel_for<Iterator> Sentinel>
constexpr auto deserialize_custom_type(Iterator begin, Sentinel end) {
    static constexpr auto fn_and_type = [] consteval {
        auto deserializers = std::ranges::to<std::vector>(members_of(^^Serializer, std::meta::access_context::current()) | std::views::filter([](auto member) consteval {
            if (is_special_member_function(member)) {
                return false;
            }
            
            if (!is_static_member(member)) {
                return false;
            }
            
            if (identifier_of(member) != "deserialize") {
                return false;
            }

            return true;
        }));

        if (deserializers.size() != 1) {
            throw CTException{"None or too many members 'deserialize' found in serializer"};
        }

        auto parameters = parameters_of(deserializers.front());
        if (parameters.size() != 1) {
            throw CTException{"'deserialize' must have exactly one parameter"};
        }

        return std::pair{
            deserializers.front(),
            parameters.front(),
        };
    }();

    constexpr auto ptype = type_of(fn_and_type.second);
    using T = [:ptype:];
    auto [value, pos] = deserialize<std::remove_cvref_t<T>>(begin, end);

    auto function = [:fn_and_type.first:];
    return std::pair{
        function(value),
        pos,
    };
}

}

template<concepts::CustomType T, typename Iterator, std::sentinel_for<Iterator> Sentinel>
constexpr std::pair<T, Iterator> deserialize(Iterator begin, Sentinel end) {
    using namespace detail;
    constexpr auto annotation = get_unique_annotation(^^T, ^^CustomType);
    constexpr auto template_arg = template_arguments_of(type_of(annotation)).front();
    if constexpr (template_arg == ^^void) {
        return deserialize_custom_type<T>(begin, end);
    } else {
        using Serializer = [:template_arg:];
        return deserialize_custom_type<Serializer>(begin, end);
    }
}

}