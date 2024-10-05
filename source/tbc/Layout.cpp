#include "tbc/Layout.h"

#include <unordered_map>

namespace ngl::tbc {
Layout::Layout(std::vector<Side> structure) : structure_{structure} {
  std::unordered_map<std::size_t, std::size_t> owner_counts;
  for (auto &side : structure_) {
    for (std::size_t i = 0; i < side.size(); i++) {
      auto &slot = side.slot(i);
      if (owner_counts.count(slot.owner) == 0) {
        owner_counts.insert({slot.owner, std::size_t{0}});
      }
      slot.index = owner_counts.at(slot.owner);
      owner_counts.at(slot.owner)++;
    }
  }
}

[[nodiscard]] const Slot &Layout::GetSlot(Slot::Index i) const {
  return structure_.at(i.side).slot(i.slot);
}
} // namespace ngl::tbc