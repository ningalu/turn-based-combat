#include <catch2/catch_test_macros.hpp>

#include <stdexcept>
#include <string>

#include "tbc/util/layout/RPGLayout.hpp"

TEST_CASE("Slots are allocated indices correctly", "[RPGLayout]") {

  ngl::tbc::RPGLayout<int> l;
  l.push_back(std::vector<int>{0});
  l.push_back(std::vector<int>{1});

  CHECK(l.at(ngl::tbc::RPGLayoutIndex{0, 0}) == 0U);
  CHECK(l.at(ngl::tbc::RPGLayoutIndex{1, 0}) == 1U);

  ngl::tbc::RPGLayout<int> l2{
    std::vector<std::vector<int>>{
      std::vector<int>{
        0, 1, 2
      },
      std::vector<int>{
        3
      }
    }
  };
  CHECK(l2.at(ngl::tbc::RPGLayoutIndex{0, 0}) == 0);
  CHECK(l2.at(ngl::tbc::RPGLayoutIndex{0, 1}) == 1);
  CHECK(l2.at(ngl::tbc::RPGLayoutIndex{0, 2}) == 2);
  CHECK(l2.at(ngl::tbc::RPGLayoutIndex{1, 0}) == 3);

  CHECK_THROWS(l2.at(ngl::tbc::RPGLayoutIndex{99, 99}));
}
