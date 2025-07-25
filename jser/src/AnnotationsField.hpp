#pragma once

#include "CTString.hpp"

/*
TODO:
custom_field(<serializer>)
this serializer only supports being called with an object context
the object context is always the one of the enclosing object
-> the serializer can add new fields
-> the serializer can add any number of subobjects, each with any number of fields

(maybe custom_field is not a good name, what's actually custom is the serialization of the data member)
*/
namespace jser {

namespace detail {

template<size_t capacity>
struct NameAnnotation: CTString<capacity> {
    using CTString<capacity>::CTString;
};

struct EmbedAnnotation {
    bool value;
};

template<typename Serializer>
struct CustomFieldAnnotation {};

}

inline namespace annotations {

template<size_t capacity>
consteval auto name(const char(&literal)[capacity]) {
    return detail::NameAnnotation<capacity>{literal};
}

consteval auto embed(bool value = true) {
    return detail::EmbedAnnotation{value};
}

template<typename Serializer>
consteval auto custom_field() {
    return detail::CustomFieldAnnotation<Serializer>{};
}

}

}