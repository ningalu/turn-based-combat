#ifndef TBC_LAYOUT_H
#define TBC_LAYOUT_H

#include <cstdint>
#include <vector>

#include "tbc/Side.h"

namespace ngl::tbc {
class Layout {
public:
  Layout(std::vector<Side> structure);

protected:
  std::vector<Side> structure_;
};
} // namespace ngl::tbc

#endif