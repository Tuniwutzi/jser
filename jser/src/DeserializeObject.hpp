#pragma once

#include "ObjectDeserializationContext.hpp"

#include <meta>

namespace jser {

template<typename T, typename Iterator, typename Sentinel>
constexpr std::pair<T, Iterator> deserialize(Iterator begin, Sentinel end) {
    T value;

    std::bitset<detail::ObjectInfo<T>::fields.size()> found{0};
    ObjectDeserializationContext context{std::move(begin), std::move(end)};
    while (context.next([&value, &found](std::string name, Iterator pos, Sentinel end) constexpr -> Iterator {
        size_t index = 0;
        template for (constexpr auto field: detail::ObjectInfo<T>::fields) {
            if (identifier_of(field) != name) {
                ++index;
                continue;
            }

            found.set(index);

            using TField = [:type_of(field):];
            std::tie(value.[:field:], pos) = deserialize<TField>(pos, end);
            
            return pos;
        }

        throw CTException{"Found unexpected field in json"};
    })) {}

    if (!found.all()) {
        throw CTException{"Did not find all expected fields in the json"};
    }

    return {std::move(value), context.finalize()};
    // return {std::move(value), begin};
}

}