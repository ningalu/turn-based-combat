#ifndef TBC_SLOT_H
#define TBC_SLOT_H

#include <cstddef>
#include <optional>

namespace ngl::tbc {
struct Slot {
  explicit Slot(std::size_t owner);

  std::size_t party;

  // TODO: is leaving this nullopt a reasonable way of signaling that the user doesn't care about manually setting slot indices?
  // maybe make it a constructor flag
  std::size_t index;

  struct Index {
    Index(std::size_t side_, std::size_t slot_);
    [[nodiscard]] bool operator==(const Index &) const = default;
    std::size_t side, slot;
  };
};
} // namespace ngl::tbc

#endif