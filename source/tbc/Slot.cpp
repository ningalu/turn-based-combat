#include "tbc/Slot.h"

namespace ngl::tbc {
Slot::Slot(std::size_t owner) : party{owner}, index{0} {}

Slot::Index::Index(std::size_t side_, std::size_t slot_) : side{side_}, slot{slot_} {}

} // namespace ngl::tbc