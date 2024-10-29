#ifndef TBC_UTIL_LAYOUT_RPGLAYOUT_HPP
#define TBC_UTIL_LAYOUT_RPGLAYOUT_HPP

#include <cstdint>
#include <vector>

namespace ngl::tbc {

struct RPGLayoutIndex {
  std::size_t side, slot;
  [[nodiscard]] bool operator==(const RPGLayoutIndex &) const noexcept = default;
};

// TODO: theres gotta be ab etter name for this
template <typename TData>
class RPGLayout : public std::vector<std::vector<TData>> {
  using Self = std::vector<std::vector<TData>>;

public:
  // TODO: is there any data Slot needs to track other than player defined stuff?
  using Slot  = TData;
  using Side  = std::vector<Slot>;
  using Index = RPGLayoutIndex;

  [[nodiscard]] const TData &at(RPGLayoutIndex i) const { return Self::at(i.side).at(i.slot); }
  [[nodiscard]] TData &at(RPGLayoutIndex i) { return Self::at(i.side).at(i.slot); }

  [[nodiscard]] const TData &at(std::size_t side, std::size_t slot) const { return Self::at(side).at(slot); }
  [[nodiscard]] TData &at(std::size_t side, std::size_t slot) { return Self::at(side).at(slot); }
};
} // namespace ngl::tbc

#endif