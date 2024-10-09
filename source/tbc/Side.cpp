#include "tbc/Side.h"

namespace ngl::tbc {
Side::Side(std::vector<Slot> slots) : slots_{slots} {}

[[nodiscard]] const std::vector<Slot> &Side::slots() const {
  return slots_;
}

[[nodiscard]] Slot &Side::slot(std::size_t i) {
  return const_cast<Slot &>(const_cast<const Side *>(this)->slot(i));
}

[[nodiscard]] const Slot &Side::slot(std::size_t i) const {
  return slots_.at(i);
}

[[nodiscard]] std::size_t Side::size() const {
  return slots_.size();
}

[[nodiscard]] std::optional<std::size_t> Side::FindSlot(std::size_t owner, std::size_t unit) const {
  for (std::size_t i = 0; i < slots_.size(); i++) {
    if ((slots_.at(i).owner == owner) && (slots_.at(i).index == unit)) {
      return i;
    }
  }
  return std::nullopt;
}

} // namespace ngl::tbc