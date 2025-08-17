#include "catch2/catch_template_test_macros.hpp"
#include "catch2/catch_test_macros.hpp"
#include <catch2/catch_all.hpp>

#include <Serialize.hpp>

using Integrals = std::tuple<int8_t, uint8_t, int16_t, uint16_t, int32_t, uint32_t, int64_t, uint64_t>;

TEMPLATE_LIST_TEST_CASE("Serialize integral", "[serialize]", Integrals) {
    static_assert(jser::serialize<TestType>(123) == "123");
    std::string integral = jser::serialize<TestType>(106);
    REQUIRE(integral == "106");
}

TEST_CASE("Serialize bool", "[serialize]") {
    static_assert(jser::serialize(true) == "true");
    std::string value = jser::serialize<bool>(false);
    REQUIRE(value == "false");
}

TEST_CASE("Serialize string", "[serialize]") {
    static_assert(jser::serialize("foo") == "\"foo\"");
    std::string value = jser::serialize("bar");
    REQUIRE(value == "\"bar\"");
}

TEST_CASE("Serialize ranges", "[serialize]") {
    static_assert(jser::serialize(std::array{true, false}) == "[true,false]");
    static_assert(jser::serialize(std::array<bool, 0>{}) == "[]");
}