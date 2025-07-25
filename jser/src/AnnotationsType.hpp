#pragma once

namespace jscheme {

namespace detail {

template<typename Serializer>
struct CustomType {};

}

inline namespace annotations {

template<typename Serializer = void>
consteval auto custom_type() {
    return detail::CustomType<Serializer>{};
}

}

}