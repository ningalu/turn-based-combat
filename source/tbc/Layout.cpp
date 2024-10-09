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

[[nodiscard]] const std::vector<Side> &Layout::sides() const { return structure_; }

[[nodiscard]] const Slot &Layout::GetSlot(Slot::Index i) const {
  return structure_.at(i.side).slot(i.slot);
}

[[nodiscard]] std::optional<Slot::Index> Layout::FindSlot(std::size_t owner, std::size_t unit) const {
  for (std::size_t i = 0; i < structure_.size(); i++) {
    const auto &side = structure_.at(i);
    const auto index = side.FindSlot(owner, unit);
    if (index.has_value()) {
      return Slot::Index{i, index.value()};
    }
  }
  return std::nullopt;
}

} // namespace ngl::tbc