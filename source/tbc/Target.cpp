#include "tbc/Target.hpp"

namespace ngl::tbc {
Target::Target(SlotTarget t) : payload{t} {}
Target::Target(SideTarget t) : payload{t} {}
Target::Target(BattleTarget t) : payload{t} {}
} // namespace ngl::tbc