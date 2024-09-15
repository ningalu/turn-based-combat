#ifndef TBC_SLOT_H
#define TBC_SLOT_H

#include <cstddef>
#include <optional>

namespace ngl::tbc {
struct Slot {
  Slot(std::size_t owner_, std::optional<std::size_t> index_ = std::nullopt);
  std::size_t owner;

  // TODO: is leaving this nullopt a reasonable way of signaling that the user doesn't care about manually setting slot indices?
  // maybe make it a constructor flag
  std::optional<std::size_t> index;

  struct Index {
    Index(std::size_t side_, std::size_t slot_);
    std::size_t side, slot;
  };
};
} // namespace ngl::tbc

#endif