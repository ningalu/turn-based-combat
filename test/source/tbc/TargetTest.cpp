#include <catch2/catch_test_macros.hpp>

#include <stdexcept>
#include <string>

#include "tbc/Target.hpp"

TEST_CASE("Targets are filtered correctly", "[Target]") {
  std::vector<ngl::tbc::Target> homogenous_targets;
  const auto initial_slot = ngl::tbc::Slot::Index{0, 1};
  const auto initial_side = ngl::tbc::Target::SideTarget{0};
  const auto initial_bat  = ngl::tbc::Target::BattleTarget{};

  homogenous_targets.push_back(initial_slot);
  homogenous_targets.push_back(initial_side);
  homogenous_targets.push_back(initial_bat);

  const auto slot_targets = ngl::tbc::Target::Filter<ngl::tbc::Slot::Index>(homogenous_targets);
  const auto side_targets = ngl::tbc::Target::Filter<ngl::tbc::Target::SideTarget>(homogenous_targets);
  const auto bat_targets  = ngl::tbc::Target::Filter<ngl::tbc::Target::BattleTarget>(homogenous_targets);

  CHECK(slot_targets.size() == 1);
  CHECK(slot_targets.at(0) == initial_slot);

  CHECK(side_targets.size() == 1);
  CHECK(side_targets.at(0) == initial_side);

  CHECK(bat_targets.size() == 1);
}
