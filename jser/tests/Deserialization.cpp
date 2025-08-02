#include "catch2/catch_template_test_macros.hpp"
#include "catch2/catch_test_macros.hpp"
#include <catch2/catch_all.hpp>

#include <Deserialize.hpp>

TEST_CASE("Deserialize string success", "[deserialize][success]") {
    static_assert(jser::deserialize<std::string>("\"foo\"") == "foo");
    std::string str = jser::deserialize<std::string>("\"bar\"");
    REQUIRE(str == "bar");
}

TEST_CASE("Deserialize string failure", "[deserialize][failure]") {
    REQUIRE_THROWS(jser::deserialize<std::string>(""));
    REQUIRE_THROWS(jser::deserialize<std::string>("unquoted"));
    REQUIRE_THROWS(jser::deserialize<std::string>("\"unterminated"));
    REQUIRE_THROWS(jser::deserialize<std::string>("unstarted\""));
    REQUIRE_THROWS(jser::deserialize<std::string>(" \"unexpected start\""));
}

using Integrals = std::tuple<int8_t, uint8_t, int16_t, uint16_t, int32_t, uint32_t, int64_t, uint64_t>;

TEMPLATE_LIST_TEST_CASE("Deserialize integral success", "[deserialize][success]", Integrals) {
    static_assert(jser::deserialize<TestType>("123") == 123);
    TestType integral = jser::deserialize<TestType>("0106");
    REQUIRE(integral == 106);
}

TEMPLATE_LIST_TEST_CASE("Deserialize integral failure", "[deserialize][failure]", Integrals) {
    REQUIRE_THROWS(jser::deserialize<TestType>("nonnumeric"));
    REQUIRE_THROWS(jser::deserialize<TestType>(" 123"));
    REQUIRE_THROWS(jser::deserialize<TestType>("0x123"));
    REQUIRE_THROWS(jser::deserialize<TestType>("99999999999999999999999999999999999999999"));
    REQUIRE_THROWS(jser::deserialize<TestType>("1,5"));
    REQUIRE_THROWS(jser::deserialize<TestType>("1.5"));
}
