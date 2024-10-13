#ifndef TBC_LAYOUT_H
#define TBC_LAYOUT_H

#include <cstdint>
#include <optional>
#include <vector>

#include "tbc/Side.h"

namespace ngl::tbc {
class Layout {
public:
  Layout() = default;
  Layout(std::vector<Side> structure);

  [[nodiscard]] const std::vector<Side> &sides() const;

  [[nodiscard]] const Slot &GetSlot(Slot::Index i) const;

  [[nodiscard]] std::optional<Slot::Index> FindSlot(std::size_t owner, std::size_t unit) const;

protected:
  std::vector<Side> structure_;
};
} // namespace ngl::tbc

#endif