#include <catch2/catch_test_macros.hpp>

#include <stdexcept>
#include <string>

#include "tbc/Layout.h"

TEST_CASE("Slots are allocated indices correctly", "[Layout]") {

  ngl::tbc::Layout l{
    std::vector<ngl::tbc::Side>{
      ngl::tbc::Side{std::vector<ngl::tbc::Slot>{ngl::tbc::Slot{0}}},
      ngl::tbc::Side{std::vector<ngl::tbc::Slot>{ngl::tbc::Slot{1}}}
    }
  };

  CHECK(l.GetSlot(ngl::tbc::Slot::Index{0, 0}).index == 0U);
  CHECK(l.GetSlot(ngl::tbc::Slot::Index{1, 0}).index == 0U);

  ngl::tbc::Layout l2{
    std::vector<ngl::tbc::Side>{
      ngl::tbc::Side{std::vector<ngl::tbc::Slot>{
        ngl::tbc::Slot{0},
        ngl::tbc::Slot{0},
        ngl::tbc::Slot{0}
      }},
      ngl::tbc::Side{std::vector<ngl::tbc::Slot>{
        ngl::tbc::Slot{1}
      }}
    }
  };
  CHECK(l2.GetSlot(ngl::tbc::Slot::Index{0, 2}).index == 2U);
  CHECK(l2.GetSlot(ngl::tbc::Slot::Index{1, 0}).index == 0U);
}
