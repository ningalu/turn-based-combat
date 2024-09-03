#include "tbc/Side.h"

namespace ngl::tbc {
Side::Side(std::vector<Slot> slots) : slots_{slots} {}

[[nodiscard]] Slot Side::GetSlot(std::size_t i) {
  return slots_.at(i);
}
} // namespace ngl::tbc