#include <catch2/catch_test_macros.hpp>

#include "tbc/lib.hpp"

TEST_CASE("Name is ningalu-turn-based-combat-2", "[library]") {
  auto const lib = library{};
  REQUIRE(lib.name == "ningalu-turn-based-combat-2");
}
