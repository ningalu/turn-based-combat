#ifndef TBC_SIDE_H
#define TBC_SIDE_H

#include <vector>

#include "tbc/Slot.h"

namespace ngl::tbc {
class Side {
public:
  Side(std::vector<Slot> slots);

  [[nodiscard]] Slot &slot(std::size_t i);
  [[nodiscard]] const Slot &slot(std::size_t i) const;
  [[nodiscard]] std::size_t size() const;

protected:
  std::vector<Slot> slots_;
};
} // namespace ngl::tbc

#endif