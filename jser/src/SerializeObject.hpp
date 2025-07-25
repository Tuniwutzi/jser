#pragma once

#include "ObjectSerializationContext.hpp"

namespace jscheme {

namespace detail {

struct FieldAnnotations {
    std::string_view name;
    bool embed = false;
};

consteval std::optional<std::meta::info> get_custom_field_serializer(std::meta::info field) {
    if (auto custom_field_annotation = get_unique_optional_annotation(field, ^^detail::CustomFieldAnnotation)) {
        return template_arguments_of(type_of(*custom_field_annotation)).front();
    }

    return std::nullopt;
}

template<std::meta::info field>
consteval FieldAnnotations get_default_field_annotations() {
    auto name = [] consteval {
        constexpr auto name_annotation = get_unique_optional_annotation(field, ^^NameAnnotation);
        if constexpr (name_annotation) {
            return [: *name_annotation :].string_view();
        } else {
            return identifier_of(field);
        }
    }();

    auto embed = [] consteval {
        if constexpr (constexpr auto embed_annotation = get_unique_optional_annotation(field, ^^EmbedAnnotation)) {
            return [: *embed_annotation :].value;
        } else {
            return false;
        }
    }();
    return {
        .name = name,
        .embed = embed,
    };
}

}

template<typename T, typename OutIt>
requires(!concepts::String<T> && !std::integral<T> && !concepts::CustomType<T>)
constexpr OutIt serialize(const T& value, OutIt iterator);

namespace detail {

template<typename T, typename Out>
constexpr void serialize_fields(const T& value, ObjectSerializationContext<Out>& context) {
    template for (constexpr auto member:
                  std::define_static_array(nonstatic_data_members_of(^^T, std::meta::access_context::unchecked()))) {
        constexpr auto custom_serializer = get_custom_field_serializer(member);
        if constexpr (custom_serializer) {
            [:*custom_serializer:]::serialize(value.[:member:], context);
        } else {
            constexpr auto annotations = get_default_field_annotations<member>();

            if constexpr (annotations.embed) {
                serialize_fields(value.[:member:], context);
            } else {
                context.add_field(annotations.name, value.[:member:]);
            }
        }
    }
}

}

template<typename T, typename OutIt>
requires(!concepts::String<T> && !std::integral<T> && !concepts::CustomType<T>)
constexpr OutIt serialize(const T& value, OutIt iterator) {
    ObjectSerializationContext context{iterator};
    detail::serialize_fields(value, context);
    return context.finalize();
}


}