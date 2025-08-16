#pragma once

#include "DeserializeBasic.hpp"
#include "CTException.hpp"

#include <string_view>
#include <type_traits>

namespace jser {

template<typename Iterator, typename Sentinel>
class ObjectDeserializationContext {
public:
    constexpr ObjectDeserializationContext(Iterator begin, Sentinel end)
    :_begin{std::move(begin)}
    ,_end{std::move(end)}
    {
        auto c = _consume();
        if (c != '{') {
            throw CTException{"Unexpected character at beginning of json object"};
        }
        _skip_whitespace();
    }

    /**
     * @brief 
     * 
     * @tparam Deserializer 
     * @param deserializer 
     * @return true if a field was present and the deserializer was called, false otherwise
     */
    template<typename Deserializer>
    requires std::is_invocable_r_v<Iterator, Deserializer, std::string, Iterator, Sentinel>
    constexpr bool next(Deserializer&& deserializer) {
        auto c = _peek();
        if (c == '\"') {
            auto [name, pos] = deserialize<std::string>(_begin, _end);
            _begin = pos;
            _skip_whitespace();
            if (_consume() != ':') {
                throw CTException{"Missing ':' after fieldname"};
            }
            _skip_whitespace();
            _begin = std::forward<Deserializer>(deserializer)(std::move(name), std::move(_begin), _end);
            _skip_whitespace();
            auto c = _peek();
            if (c == ',') {
                ++_begin;
            } else if (c != '}') {
                throw CTException{"Invalid character following json field"};
            }
            _skip_whitespace();
            return true;
        } else {
            return false;
        }
    }

    constexpr Iterator finalize() {
        if (_consume() != '}') {
            throw CTException{"Invalid character found when finalizing json object"};
        }
        
        return _begin;
    }

private:
    constexpr void _skip_whitespace() {
        while (_begin != _end) {
            switch (*_begin) {
                case ' ':
                    ++_begin;
                    continue;
                default:
                    return;
            }
        }
    }
    constexpr char _peek() {
        if (_begin != _end) {
            return *_begin;
        } else {
            throw CTException{"Unexpected end of json object"};
        }
    }
    constexpr char _consume() {
        auto c = _peek();
        ++_begin;
        return c;
    }
    // constexpr char _skip_and_consume() {
    //     _skip_whitespace();
    //     return _consume();
    // }

    Iterator _begin;
    Sentinel _end;
};

}