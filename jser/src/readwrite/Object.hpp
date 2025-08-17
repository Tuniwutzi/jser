#pragma once

#include "Helpers.hpp"

#include <bitset>
#include <ranges>
#include <stdexcept>
#include <type_traits>
#include <meta>

namespace jser::readwrite {

namespace detail {

namespace concepts {

// A bit of a crutch; we recognize objects when they're a class and not a range
template<typename Candidate>
concept Object = std::is_class_v<Candidate> && !std::ranges::range<Candidate>;

}

static constexpr auto access_context = std::meta::access_context::current();

template<typename T>
struct Storage {
};

}

template<detail::concepts::Object O>
constexpr auto read(auto begin, auto end) {
    static constexpr auto fields = [] {
        auto members = nonstatic_data_members_of(^^O, detail::access_context);
        std::ranges::sort(members, std::less<>{}, std::meta::identifier_of);
        return std::define_static_array(members);
    }();
    // Workaround for compiler issue; it won't let me get fields.size() at runtime
    static constexpr auto fields_size = fields.size();

    std::bitset<fields.size()> found(0);

    auto parse_field_name = [&] constexpr -> std::string {
        std::string name;
        std::tie(name, begin) = deserialize<std::string>(begin, end);
        begin = detail::skip_whitespace(begin, end);
        if (begin == end) {
            throw std::runtime_error{"Unexpected end to json object string after field name"};
        }

        if (*begin != ':') {
            throw std::runtime_error{"Expected ':' after field name when parsing json object"};
        }
        begin = detail::skip_whitespace(begin + 1, end);
        return name;
    };

    auto parse_fields = [&](O& object) constexpr {
        while (true) {
            auto name = parse_field_name();

            // TODO: the fields are ordered, so binary search might be nice
            size_t i = 0;
            template for (constexpr auto& field: fields) {
                if (identifier_of(field) != name) {
                    ++i;
                    continue;
                }

                if (found[i]) {
                    throw std::runtime_error{"Field must not occur twice in json object"};
                }

                found[i] = true;

                using T = [:type_of(field):];
                std::tie(object.[:field:], begin) = deserialize<T>(begin, end);
                break;
            }

            if (i == fields_size) {
                throw std::runtime_error{"Json object contains unexpected field"};
            }

            begin = detail::skip_whitespace(begin, end);
            if (begin == end) {
                throw std::runtime_error{"Unexpected end of json object"};
            }

            if (*begin == ',') {
                begin = detail::skip_whitespace(begin + 1, end);
                continue;
            } else {
                break;
            }
        }
    };

    if (end - begin < 2) {
        throw std::runtime_error{"Too few characters to parse json object"};
    }

    if (*begin != '{') {
        throw std::runtime_error{"Json object must begin with '{'"};
    }

    begin = detail::skip_whitespace(begin + 1, end);
    if (begin == end) {
        throw std::runtime_error{"Unexpected end of json object"};
    }

    O object;
    if (*begin != '}') {
        parse_fields(object);
    }

    if (*begin != '}') {
        throw std::runtime_error{"Json object must end with '}'"};
    }
    ++begin;

    if (!found.all()) {
        throw std::runtime_error{"Missing field in json object"};
    }

    return std::pair {
        std::move(object),
        std::move(begin),
    };
}

}