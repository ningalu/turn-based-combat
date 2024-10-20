#ifndef TBC_TARGET_H
#define TBC_TARGET_H

#include <cstddef>
#include <variant>
#include <vector>

#include "tbc/Slot.h"

namespace ngl::tbc {
struct Target {
  using SlotTarget = Slot::Index;
  struct SideTarget {
    std::size_t side;
    [[nodiscard]] bool operator==(const SideTarget &) const noexcept = default;
  };
  struct BattleTarget {
  };
  struct PartyTarget {
    std::size_t member;
  };

  explicit Target(SlotTarget t);
  explicit Target(SideTarget t);
  explicit Target(BattleTarget t);
  explicit Target(PartyTarget t);

  template <typename T>
  [[nodiscard]] static std::vector<T> Filter(const std::vector<Target> &t) {
    std::vector<T> out;
    for (const auto &target : t) {
      std::visit([&](auto &&visited) {
        using P = std::decay_t<decltype(visited)>;
        if constexpr (std::is_same_v<P, T>) {
          out.push_back(visited);
        }
      },
                 target.payload);
    }
    return out;
  }

  std::variant<SlotTarget, SideTarget, BattleTarget, PartyTarget> payload;
};
} // namespace ngl::tbc

#endif