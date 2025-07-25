#include <print>
#include <meta>
#include <iostream>
#include <string>

#include "Serialize.hpp"
#include "Deserialize.hpp"



struct AreaSerializer;

struct [[=jser::custom_type<AreaSerializer>()]] Area {
    std::string post_number;
    std::string city;
};

struct AreaSerializer {
    static constexpr std::string serialize(const Area& area) {
        return area.post_number + " " + area.city;
    }
};

struct Address {
    std::string street;
    std::string number;
    Area area;
};

struct [[=jser::custom_type<>()]] Name {
    std::string first_name;
    std::string last_name;

    static constexpr std::string serialize(const Name& value) {
        return value.first_name + " " + value.last_name;
    }

    static constexpr Name deserialize(const std::string& raw) {
        auto pos = raw.find(' ');
        if (pos == std::string::npos) {
            throw std::runtime_error{"Invalid name"};
        }

        return {
            .first_name { raw.substr(0, pos) },
            .last_name { raw.substr(pos + 1) },
        };
    }
};

struct PhoneNumber {
    std::string country_code;
    std::string number;

    template<typename Iterator>
    static constexpr void serialize(const PhoneNumber& value, jser::ObjectSerializationContext<Iterator>& context) {
        context.add_object_field("PhoneNumber", [&](auto& context) constexpr {
            context.add_field("CountryCode", value.country_code);
            context.add_field("Number", value.number);
        });
    }
};

struct Person {
    [[=jser::name("$test")]] Name name;
    std::uint8_t age;
    [[=jser::embed(true)]] Address address;

    [[=jser::custom_field<PhoneNumber>()]] PhoneNumber phone_number;
};


void test_serialization() {
    static constexpr auto serializedCT = std::define_static_string(jser::serialize(Person{
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

    auto serializedRT = jser::serialize(*new Person{{"fn", "ln"}, 17});
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

    static constexpr auto str = jser::deserialize<std::string>(json_string);
    static_assert(str == "\"\\ hey!");

    try {
        auto _ = jser::deserialize<std::string>(json_broken_string);
        std::cerr << "Should get error on broken string" << std::endl;
    } catch (...) {
    }

    static constexpr std::string_view json_int = "123";
    static constexpr auto intval = jser::deserialize<int>(json_int);
    static_assert(intval == 123);

    static constexpr std::string_view json_name = "\"FirstName LastName\"";
    static constexpr auto name = jser::deserialize<Name>(json_name);
    static_assert(name.first_name == "FirstName" && name.last_name == "LastName");
}

int main() {
    test_serialization();
    test_deserialization();

    return 0;
}