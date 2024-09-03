#ifndef TBC_TARGET_H
#define TBC_TARGET_H

#include <cstddef>
#include <variant>

#include "tbc/Slot.h"

namespace ngl::tbc {
struct Target {
  using SlotTarget = Slot::Index;
  using SideTarget = std::size_t;
  struct BattleTarget {};

  Target(SlotTarget t);
  Target(SideTarget t);
  Target(BattleTarget t);

  std::variant<SlotTarget, SideTarget, BattleTarget> payload;
};
} // namespace ngl::tbc

#endif