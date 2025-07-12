#pragma once

#include "Serialize.hpp"
#include "Validation.hpp"
#include "Serialization.hpp"

#include <string>
#include <meta>

namespace jscheme {

namespace generation {

struct ValidationSerializer;

enum class [[=jscheme::serialization::custom_serializer<ValidationSerializer>]] ValidationType {
    String,
    Integer,
    Object,
};

struct ValidationTypeSerializer {
    static constexpr std::string_view string_type = "string";
    static constexpr std::string_view integer_type = "integer";
    static constexpr std::string_view object_type = "object";
    static constexpr auto serialize(ValidationType type, auto iterator) {
        using enum ValidationType;
        switch (type) {
            case String:
            return serialize(string_type, iterator);
            case Integer:
            return serialize(integer_type, iterator);
            case Object:
            return serialize(object_type, iterator);
            default:
            throw std::runtime_error{"Unknown validation type"};
        }
    }
};

consteval ValidationType get_validation_type(std::meta::info type) {
    if (is_same_type(type, ^^bool)) {
        throw std::runtime_error{"Bool not supported yet"};
    } else if (is_integral_type(type)) {
        return ValidationType::Integer;
    } else if (is_same_type(type, ^^std::string) || is_same_type(type, ^^std::string_view)) {
        return ValidationType::String;
    } else {
        return ValidationType::Object;
    }
}

struct ValidationSerializer {
    constexpr static auto operator()(const auto& value, auto iterator) {
        *iterator = 'o';
        ++iterator;
        return iterator;
    }
};

struct Value;

struct ObjectValidation {
    std::vector<std::pair<std::string, std::reference_wrapper<Value>>> fields;
};

struct [[=jscheme::serialization::custom_serializer<ValidationSerializer>]] Validation: std::variant<std::monostate, ObjectValidation> {
};

struct Value {
    std::string description;
    [[=jscheme::serialization::embed]] Validation validation;
};

struct Scheme {
    static constexpr std::string_view schema_name = "$schema";
    [[=jscheme::serialization::Name(^^schema_name)]] std::string_view schema;

    static constexpr std::string_view id_name = "$id";
    [[=jscheme::serialization::Name(^^id_name)]] std::string id;

    std::string title;
    [[=jscheme::serialization::embed]] Value value;
};

// consteval std::vector<std::pair<std::string, Value>> generate_properties(std::meta::info aggregate) {
//     std::vector<std::pair<std::string, Value>> properties;
//     for (auto member: nonstatic_data_members_of(aggregate, context)) {
//         auto& [name, value] = properties.emplace_back();
//         name = serialization::get_name(member);
//         value.description = "";
//         if (get_validation_type(type_of(member)) == ValidationType::Object) {
//             // ??
//         }
//     }
//     return properties;
// }

}

consteval std::string_view generate(std::string_view id, std::string_view description, std::meta::info type) {
    auto context = std::meta::access_context::unchecked();

    generation::Scheme scheme {
        .schema = "https://json-schema.org/draft/2020-12/schema",
        .id {id},
        .title {serialization::get_name(type)},
        .value {
            .description {description},
            .validation {}
        },
    };

    // TODO: support other types!
    generation::ObjectValidation& object_validation = scheme.value.validation.emplace<generation::ObjectValidation>();
    std::vector<generation::Value> value_storage{};
    for (auto member: nonstatic_data_members_of(type, context)) {
        generation::Value& value = value_storage.emplace_back();
        value.description = "";
        object_validation.fields.emplace_back(identifier_of(member), std::ref(value));
    }

    return std::define_static_string(serialize(scheme));
}

}