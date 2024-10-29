#ifndef TBC_UTIL_LAYOUT_GRIDLAYOUT
#define TBC_UTIL_LAYOUT_GRIDLAYOUT

#include <array>
#include <cstdint>

namespace ngl::tbc {

struct GridLayoutIndex {
  std::size_t x, y;
  [[nodiscard]] bool operator==(const GridLayoutIndex &) const noexcept = default;
};

template <typename TData, std::size_t W, std::size_t H = W>
class GridLayout : public std::array<std::array<TData, W>, H> {
  using Self = std::array<std::array<TData, W>, H>;

public:
  using Index = GridLayoutIndex;

  [[nodiscard]] const TData &at(GridLayoutIndex i) const { return Self::at(i.y).at(i.x); }
  [[nodiscard]] TData &at(GridLayoutIndex i) { return Self::at(i.y).at(i.x); }

  [[nodiscard]] const TData &at(std::size_t x, std::size_t y) const { return Self::at(y).at(x); }
  [[nodiscard]] TData &at(std::size_t x, std::size_t y) { return Self::at(y).at(x); }
};
} // namespace ngl::tbc

#endif