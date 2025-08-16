#pragma once

#include "detail/Annotations.hpp"

#include <concepts>

namespace jser::concepts {

template<typename Candidate>
concept String = std::same_as<Candidate, std::string> || std::same_as<Candidate, std::string_view>;

template<typename Candidate>
concept CustomType = detail::get_unique_optional_annotation(^^Candidate, ^^detail::CustomType).has_value();

}