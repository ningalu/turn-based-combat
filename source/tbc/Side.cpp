#include "tbc/Side.h"

namespace ngl::tbc {
Side::Side(std::vector<Slot> slots) : slots_{slots} {}

[[nodiscard]] Slot &Side::slot(std::size_t i) {
  return const_cast<Slot &>(const_cast<const Side *>(this)->slot(i));
}

[[nodiscard]] const Slot &Side::slot(std::size_t i) const {
  return slots_.at(i);
}

[[nodiscard]] std::size_t Side::size() const {
  return slots_.size();
}
} // namespace ngl::tbc