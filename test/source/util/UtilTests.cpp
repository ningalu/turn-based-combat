#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <functional>
#include <variant>

#include "util/oneof.hpp"

TEST_CASE("Variants can be matched", "[match]") {
  std::variant<int, double> v;
  v = 1;
  ngl::match(
    v,
    []([[maybe_unused]] int i) { CHECK(true); },
    []([[maybe_unused]] double d) { CHECK(false); }
  );

  v = 1.0;
  ngl::match(
    v,
    []([[maybe_unused]] int i) { CHECK(false); },
    []([[maybe_unused]] double d) { CHECK(true); }
  );
}

TEST_CASE("Active value of sum type can be matched to", "[oneof]") {
  ngl::oneof<int, double> sum;
  sum = 1;
  sum.match(
    []([[maybe_unused]] int i) { CHECK(true); },
    []([[maybe_unused]] double i) { CHECK(false); }
  );

  sum = 1.0;
  sum.match(
    []([[maybe_unused]] int i) { CHECK(false); },
    []([[maybe_unused]] double i) { CHECK(true); }
  );
}

TEST_CASE("Different callable types can be used as matches", "[oneof]") {
  ngl::oneof<uint8_t> sum;

  [[maybe_unused]] int mutable_value;

  sum = std::uint8_t{0};
  sum.match(
    std::function<void(uint8_t)>{
      [mutable_value]([[maybe_unused]] uint8_t u8) mutable { CHECK(true); }
    }
  );
}

TEST_CASE("Case that isn't covered throws", "[oneof]") {
  ngl::oneof<uint8_t, uint16_t> sum;

  [[maybe_unused]] int mutable_value;

  sum = std::uint16_t{0};
  CHECK_THROWS(sum.match(
    std::function<void(uint8_t)>{
      [mutable_value]([[maybe_unused]] uint8_t u8) mutable { CHECK(true); }
    }
  ));
}
