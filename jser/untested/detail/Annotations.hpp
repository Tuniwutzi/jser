#pragma once

#include "../CTException.hpp"

#include <meta>
#include <vector>
#include <optional>
#include <ranges>

namespace jser::detail {

consteval std::vector<std::meta::info> get_annotations(std::meta::info annotatable, std::meta::info annotation_kind) {
    return std::ranges::to<std::vector>(annotations_of(annotatable) | std::views::filter([&](auto annotation) consteval {
        auto annotation_type = type_of(annotation);
        if (is_template(annotation_kind)) {
            if (has_template_arguments(annotation_type)) {
                return template_of(annotation_type) == annotation_kind;
            } else {
                return false;
            }
        } else if (is_type(annotation_kind)) {
            return annotation_type == annotation_kind;
        } else {
            throw CTException{"Annotation kind must be template or type"};
        }
    }) | std::views::transform(std::meta::constant_of));
}

consteval std::optional<std::meta::info> get_unique_optional_annotation(std::meta::info annotatable, std::meta::info annotation_kind) {
    auto annotations = get_annotations(annotatable, annotation_kind);
    if (annotations.empty()) {
        return std::nullopt;
    } else if (annotations.size() == 1) {
        return annotations.front();
    } else {
        throw CTException{"Found more than one matching annotation"};
    }
}

consteval std::meta::info get_unique_annotation(std::meta::info annotatable, std::meta::info annotation_kind) {
    if (auto annotation = get_unique_optional_annotation(annotatable, annotation_kind)) {
        return *annotation;
    }

    throw CTException{"Expected annotation not found"};
}


}