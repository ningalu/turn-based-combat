#ifndef TBC_SIDE_H
#define TBC_SIDE_H

#include <vector>

#include "tbc/Slot.h"

namespace ngl::tbc {
class Side {
public:
  Side(std::vector<Slot> slots);

  [[nodiscard]] Slot GetSlot(std::size_t i);

protected:
  std::vector<Slot> slots_;
};
} // namespace ngl::tbc

#endif