#pragma once

#include <iterator>
#include <string>
#include <string_view>

namespace jser {

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

namespace detail {

template<typename Candidate>
concept NumberFormatter = requires(Candidate c) {
    c.format_value((float)1.);
    c.format_value((double)1.);
    c.format_value((int)1);
    // ...
};

template<typename Candidate>
concept StringFormatter = requires(Candidate c, std::string s, std::string_view sv) {
    c.format_value(s);
    c.format_value(sv);
};

template<typename Candidate>
concept BoolFormatter = requires(Candidate c) {
    c.format_value(true);
};

template<typename Candidate>
concept BasicValueFormatter = NumberFormatter<Candidate> && StringFormatter<Candidate> && BoolFormatter<Candidate>;

// template<typename Candidate>
// concept Finalizable = requires(Candidate c) {
    // Finalize should automatically be called in the destructor of the candidate
    // But also offered as a separate method
    // Finalize will return the "parent" formatter (ie if we're finalizing an object field, the formatter of the surrounding object is returned)
    // If there is no parent formatter, the out iterator is returned, now advanced to one-past the last written byte
//     { c.finalize() };
// };

template<typename Candidate>
concept InnerValueFormatter = BasicValueFormatter<Candidate> && requires(Candidate c) {
    { c.format_object() }; // Can't check for object formatter, because that would be circular
    { c.format_array() }; // Can't check for array formatter, because that would be circular
};

}

template<typename Candidate>
concept ArrayFormatter = requires(Candidate c) {
    { c.add() } -> detail::InnerValueFormatter;
    { c.finalize() };
};

template<typename Candidate>
concept ObjectFormatter = requires(Candidate c, std::string_view str) {
    { c.add_field(str) } -> detail::InnerValueFormatter;
    { c.finalize() };
};

template<typename Candidate>
concept ValueFormatter = detail::BasicValueFormatter<Candidate> && requires(Candidate c) {
    { c.format_object() } -> ObjectFormatter;
    { c.format_array() } -> ArrayFormatter;
};

template<typename Candidate>
concept Formatter = ValueFormatter<Candidate>;

}

}