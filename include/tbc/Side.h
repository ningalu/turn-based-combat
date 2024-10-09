#ifndef TBC_SIDE_H
#define TBC_SIDE_H

#include <optional>
#include <vector>

#include "tbc/Slot.h"

namespace ngl::tbc {
class Side {
public:
  Side(std::vector<Slot> slots);

  [[nodiscard]] const std::vector<Slot> &slots() const;

  [[nodiscard]] Slot &slot(std::size_t i);
  [[nodiscard]] const Slot &slot(std::size_t i) const;
  [[nodiscard]] std::size_t size() const;

  [[nodiscard]] std::optional<std::size_t> FindSlot(std::size_t owner, std::size_t unit) const;

protected:
  std::vector<Slot> slots_;
};
} // namespace ngl::tbc

#endif