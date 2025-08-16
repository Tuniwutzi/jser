#include "catch2/catch_template_test_macros.hpp"
#include "catch2/catch_test_macros.hpp"
#include <catch2/catch_all.hpp>

#include <Serialize.hpp>

using Integrals = std::tuple<int8_t, uint8_t, int16_t, uint16_t, int32_t, uint32_t, int64_t, uint64_t>;

TEMPLATE_LIST_TEST_CASE("Serialize integral success", "[serialize][success]", Integrals) {
    static_assert(jser::serialize<TestType>(123) == "123");
    std::string integral = jser::serialize<TestType>(106);
    REQUIRE(integral == "106");
}