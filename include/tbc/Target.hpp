#ifndef TBC_TARGET_H
#define TBC_TARGET_H

#include <cstddef>
#include <variant>
#include <vector>

#include "tbc/Slot.h"

namespace ngl::tbc {
struct Target {
  using SlotTarget = Slot::Index;
  using SideTarget = std::size_t;
  struct BattleTarget {};

  Target(SlotTarget t);
  Target(SideTarget t);
  Target(BattleTarget t);

  template <typename T>
  [[nodiscard]] static std::vector<T> Filter(const std::vector<Target> &t) {
    std::vector<T> out;
    for (const auto &target : t) {
      std::visit([&](auto &&payload) {
        using P = std::decay_t<decltype(payload)>;
        if constexpr (std::is_same_v<P, T>) {
          out.push_back(payload);
        }
      },
                 target.payload);
    }
    return out;
  }

  std::variant<SlotTarget, SideTarget, BattleTarget> payload;
};
} // namespace ngl::tbc

#endif