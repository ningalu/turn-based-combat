#ifndef TBC_UTIL_LAYOUT_RPGLAYOUT_HPP
#define TBC_UTIL_LAYOUT_RPGLAYOUT_HPP

#include <cstdint>
#include <optional>
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

  using Self::at;
  using Self::size;

  [[nodiscard]] const TData &at(RPGLayoutIndex i) const { return Self::at(i.side).at(i.slot); }
  [[nodiscard]] TData &at(RPGLayoutIndex i) { return Self::at(i.side).at(i.slot); }

  [[nodiscard]] const TData &at(std::size_t side, std::size_t slot) const { return Self::at(side).at(slot); }
  [[nodiscard]] TData &at(std::size_t side, std::size_t slot) { return Self::at(side).at(slot); }

  [[nodiscard]] std::optional<Index> find(const TData val) const {
    for (std::size_t i = 0; i < size(); i++) {
      for (std::size_t j = 0; j < at(i).size(); j++) {
        if (at(i).at(j) == val) {
          return Index{i, j};
        }
      }
    }
    return std::nullopt;
  }
};
} // namespace ngl::tbc

#endif