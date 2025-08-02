#pragma once

/**
 * Not a real header, I will use this to construct an example of where I want to get to.
 * As in: how should code that uses jser look like.
 */

#include "ObjectSerializationContext.hpp"
#include "crazy_formatters/FormatterConcepts.hpp"
#include <jser/Annotations.hpp>

#include <chrono>
#include <optional>
#include <tuple>
#include <string>

// Exposition only
namespace jser {

inline namespace annotations {

namespace detail {

template<typename... Settings>
class DefaultsAnnotation {
    std::tuple<Settings...> settings;
};

}

/**
 * @brief Sets the default settings for all types and fields in this scope.
 * Can be used on a namespace or a type, has no effect on a field.
 */
template<typename... Args>
consteval auto defaults(Args... args) {
    return detail::DefaultsAnnotation<Args...>{args...};
}


enum class TimepointFormat {
    UnixTimestamp,
    Default = UnixTimestamp,
};

namespace detail {

struct TimepointFormatAnnotation {
    TimepointFormat format;
};

}

consteval auto timepoint_format(TimepointFormat format) {
    return detail::TimepointFormatAnnotation { format };
}

namespace detail {

struct OmitIfEmptyAnnotation {
    bool omit;
};

}

/**
 * For a field with a nullable type this causes the field to be omitted completely if it's null, rather than "field": null.
 * Takes a boolean, because if omit_if_empty is specified as a default annotation, we must be able to unset it for specific fields.
 */
consteval auto omit_if_empty(bool omit = true) {
    return detail::OmitIfEmptyAnnotation{omit};
}

}

namespace concepts {

template<typename Candidate>
concept Bytelike = sizeof(Candidate) == 1 && std::integral<Candidate>;

template<typename Candidate>
concept BytelikeOutputIterator = std::input_or_output_iterator<Candidate> && requires(Candidate c) {
    { *c } -> Bytelike;
};

template<typename Candidate>
concept Number = !std::same_as<bool, Candidate> && (std::integral<Candidate> || std::floating_point<Candidate>);

template<typename Candidate>
concept Stringlike = std::same_as<Candidate, std::string> || std::same_as<Candidate, std::string_view>;

/**
* @brief Defines the requirements for a jser formatter.
* Notably: a formatters methods can be non-const, as it might need to cache stuff or use internal state.
*/
template<typename Candidate>
concept Formatter = requires(Candidate c, std::string_view str) {
    // After a begin_object() call, fields can be added by calling add_field(<the field name as string>); format_value(<the field value>)
    { c.begin_object() };
    { c.add_field(str) };
    { c.end_object() };

    // After begin_array(), elements can be added by calling format_value(<array element>)
    { c.begin_array() };
    { c.end_array() };

    // Must work for 3 types: bool, any number (including floating point), stringlike
    // This does NOT need support for custom types, those are handled a layer above;
    // ie any custom type ends up as one of the 5 basic json types anyway.
    { c.format_value(...) };

    // Finishes the serialization process
    // Not that important for json (can just write stuff on the fly)
    // But probably important for bson (must cache some stuff to see if it can be stored in an optimized way)
    { c.finalize() } -> BytelikeOutputIterator;
};

template<typename Candidate>
concept FormatterOptions = requires(Candidate options, char* output_iterator) {
    { jser::formatters::make_formatter(std::move(options), output_iterator) } -> Formatter;
};

/*
Example for adding nested stuff:

Formatter formatter{};
formatter.begin_object();
formatter.format_value("person");
formatter.begin_object();
formatter.format_value("name");
formatter.format_value("Peter");
formatter.end_object();
formatter.format_value("amount");
formatter.format_value(7);
formatter.end_object();

=>
{
"person": { "name": "Peter" },
"amount": 7
}

*/

}

namespace formatters {

// No options for the basic formatter
struct Basic {};

struct Pretty {
    size_t indent = 4;
};

namespace detail {
template<typename Iterator>
struct BasicFormatter {
    Iterator out;
};
template<typename Iterator>
struct PrettyFormatter {
    const Pretty options;
    Iterator out;
};
}

auto make_formatter(Basic, auto iterator) {
    return detail::BasicFormatter{iterator};
}

auto make_formatter(Pretty options, auto iterator) {
    return detail::PrettyFormatter{options, iterator};
}

}

auto serialize(const auto& value, auto iterator, concepts::FormatterOptions auto options) {
    auto serializer = formatters::make_formatter(std::move(options), iterator);
    // ...
    return serializer.finalize();
}

auto serialize(const auto& value, concepts::FormatterOptions auto options) {
    std::string str{};
    serialize(value, std::back_inserter(str), std::move(options));
    return str;
}

}

namespace [[=defaults(timepoint_format(jser::TimepointFormat::UnixTimestamp))]] shop {

struct Person {
    std::string name;
    std::chrono::system_clock::time_point birthday;
    [[=jser::omit_if_empty(/*omit = true*/)]] std::optional<std::chrono::system_clock::time_point> deathday;
};

/*
JSON:
{
"full_plate": "COE ASD 420",
"red": 0,
"blue": 255,
"green": 0
}
The parameter names and types determine the expectation.
custom_from_object might not even be necessary, we'll see.
*/
[[=jser::custom_from_object()]] constexpr LicensePlate custom_deserialize(std::string [[=jser::name("full_plate")]] plate, uint8_t red, uint8_t green, uint8_t blue) {
}

// Alternatively: ["COE ASD 420", 0, 255, 0]
// Notably, these 2 functions must not exist at the same time, otherwise "can't take reflection of overload set"
[[=jser::custom_from_array()]] constexpr LicensePlate custom_deserialize(std::string plate, uint8_t red, uint8_t green, uint8_t blue);

constexpr void custom_serialize(const LicensePlate& plate, jser::ObjectSerializationContext& ctx);
constexpr void custom_serialize(const LicensePlate& plate, jser::ArraySerializationContext& ctx);
// Or, with temporary objects (less performant):
constexpr std::unordered_map<std::string, std::variant<std::string, uint8_t>> custom_serialize(const LicensePlate&);
constexpr std::tuple<std::string, uint8_t, uint8_t, uint8_t> custom_serialize(const LicensePlate&);

// Nice idea in principle, but:
// 1st: the annotation makes it so that we need a forward decl of the struct. (need the struct to declare the functions, need the functions to declare the struct with the annotation)
// 2nd: we'll keep running into "reflection of overload set" issues with this probably. Would be nice to have the feature that the user can decide whether to deserialize from array or object, either can work.
// that's impossible with this. Also, name clashes. Users will have to keep creating little namespaces or classes namespace license_plate_serialization { ... } to keep things unambiguous
struct [[=jser::custom(^^custom_serialize, ^^custom_deserialize)]] LicensePlate {
    std::string area;
    std::string letters;
    uint16_t numbers;
    bool yellow;
};

struct Car {
    LicensePlate license_plate;
};

}