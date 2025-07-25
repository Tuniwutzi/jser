#pragma once

#include "detail/Helpers.hpp"
#include "SerializeBasic.hpp"

#include <string_view>

namespace jscheme {

template<typename Iterator>
class ObjectSerializationContext {
public:
    constexpr ObjectSerializationContext(Iterator out_iterator)
    :_out_iterator(std::move(out_iterator))
    {
        detail::write('{', _out_iterator);
    }

    ObjectSerializationContext(const ObjectSerializationContext&) = delete;
    ObjectSerializationContext& operator=(const ObjectSerializationContext&) = delete;
    
    constexpr void add_field(std::string_view name, const auto& value) {
        _add_field(name);
        _out_iterator = serialize(value, _out_iterator);
    }

    // TODO: it probably makes sense to support this in the form of a free serialize overload
    constexpr void add_object_field(std::string_view name, auto&& serializer) {
        _add_field(name);
        ObjectSerializationContext inner = {
            _out_iterator
        };
        serializer(inner);
        _out_iterator = inner.finalize();
    }

    constexpr Iterator finalize() {
        detail::write('}', _out_iterator);
        return std::move(_out_iterator);
    }

private:
    constexpr void _add_field(std::string_view name) {
        if (first_field) {
            first_field = false;
        } else {
            detail::write(',', _out_iterator);
        }

        _out_iterator = serialize(name, _out_iterator);

        detail::write(':', _out_iterator);
    }

    Iterator _out_iterator;
    bool first_field = true;
};

}