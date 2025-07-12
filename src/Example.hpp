#pragma once

#include "Annotations.hpp"
#include "ObjectSerializationContext.hpp"

#include <stdexcept>
#include <string>
#include <cstdint>

struct AreaSerializer;

struct [[=jscheme::custom_type<AreaSerializer>()]] Area {
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

struct [[=jscheme::custom_type<>()]] Name {
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
    static constexpr void serialize(const PhoneNumber& value, jscheme::ObjectSerializationContext<Iterator>& context) {
        context.add_object_field("PhoneNumber", [&](auto& context) constexpr {
            context.add_field("CountryCode", value.country_code);
            context.add_field("Number", value.number);
        });
    }
};

struct Person {
    [[=jscheme::name("$test")]] Name name;
    std::uint8_t age;
    [[=jscheme::embed(true)]] Address address;

    [[=jscheme::custom_field<PhoneNumber>()]] PhoneNumber phone_number;
};
