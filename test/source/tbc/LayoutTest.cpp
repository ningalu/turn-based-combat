#include <catch2/catch_test_macros.hpp>

#include <stdexcept>
#include <string>

#include "tbc/Layout.h"

TEST_CASE("Slots are allocated indices correctly", "[Layout]") {
  ngl::tbc::Layout l{{{{0}}, {{1}}}};

  CHECK(l.GetSlot(ngl::tbc::Slot::Index{0, 0}).index == 0u);
  CHECK(l.GetSlot(ngl::tbc::Slot::Index{1, 0}).index == 0u);

  ngl::tbc::Layout l2{{std::vector<ngl::tbc::Slot>{{0}, {0}, {0}}, {{1}}}};
  CHECK(l2.GetSlot(ngl::tbc::Slot::Index{0, 2}).index == 2u);
  CHECK(l2.GetSlot(ngl::tbc::Slot::Index{1, 0}).index == 0u);
}
