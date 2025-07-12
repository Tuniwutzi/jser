#include <print>
#include <meta>
#include <iostream>
#include <charconv>
#include <string>

#include "DeserializeBasic.hpp"
#include "Example.hpp"
// #include "Generator.hpp"
// #include "Serialization.hpp"
#include "Serialize.hpp"
#include "Deserialize.hpp"

// namespace detail {

// consteval auto generate_member_specs(std::string_view scheme) {
//     return std::array {
//         data_member_spec(^^int, {.name = "testi"}),
//     };
// }

// }

// template<std::meta::info Scheme>
// consteval auto generate_parser() {
//     // Subject for a reddit post?
//     // This MUST be static constexpr, otherwise the consteval block complains that "specs must be captured"
//     // I _THINK_ this is actually a shortcoming of the spec of consteval blocks, because IIRC they're defined as basically:
//     // static_assert([] consteval { <content of block> }(), true);
//     // IE: everything is just put into a lambda and executed in static_assert context.
//     // So of course a variable in the surrounding scope can't be used without capturing...
//     static constexpr std::array specs{detail::generate_member_specs([:Scheme:])};

//     struct S;
//     consteval {
//         std::meta::define_aggregate(^^S, specs);
//     }

//     return [](std::string_view) -> S {
//         std::println("Parser for: {}", [:Scheme:]);
//         return {};
//     };

// }

void test_serialization() {
    static constexpr auto serializedCT = std::define_static_string(jscheme::serialize(Person{
        .address {
            .area {
                .post_number = "12345",
                .city = "Cologne",
            }
        },
        .phone_number {
            .country_code = "+49",
            .number = "123",
        },
    }));
    static constexpr std::string_view expectedCT = "{\"$test\":\" \",\"age\":0,\"street\":\"\",\"number\":\"\",\"area\":\"12345 Cologne\",\"PhoneNumber\":{\"CountryCode\":\"+49\",\"Number\":\"123\"}}";
    if (serializedCT != expectedCT) {
        std::cerr << "Mismatch for compiletime serialization:" << std::endl;
        std::cerr << serializedCT << std::endl;
        std::cerr << expectedCT << std::endl;
    }

    auto serializedRT = jscheme::serialize(*new Person{{"fn", "ln"}, 17});
    static constexpr std::string_view expectedRT = "{\"$test\":\"fn ln\",\"age\":17,\"street\":\"\",\"number\":\"\",\"area\":\" \",\"PhoneNumber\":{\"CountryCode\":\"\",\"Number\":\"\"}}";
    if (serializedRT != expectedRT) {
        std::cerr << "Mismatch for runtime serialization:" << std::endl;
        std::cerr << serializedRT << std::endl;
        std::cerr << expectedRT << std::endl;
    }
}

void test_deserialization() {
    static constexpr std::string_view json_string = "\"\\\"\\\\ hey!\"";
    static constexpr std::string_view json_broken_string = "\"hey!";

    static constexpr auto str = jscheme::deserialize<std::string>(json_string);
    static_assert(str == "\"\\ hey!");

    try {
        auto _ = jscheme::deserialize<std::string>(json_broken_string);
        std::cerr << "Should get error on broken string" << std::endl;
    } catch (...) {
    }

    static constexpr std::string_view json_int = "123";
    static constexpr auto intval = jscheme::deserialize<int>(json_int);
    static_assert(intval == 123);

    static constexpr std::string_view json_name = "\"FirstName LastName\"";
    static constexpr auto name = jscheme::deserialize<Name>(json_name);
    static_assert(name.first_name == "FirstName" && name.last_name == "LastName");
}

int main() {
    test_serialization();
    test_deserialization();

    return 0;
}