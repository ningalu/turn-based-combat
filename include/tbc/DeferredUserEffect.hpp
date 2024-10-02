#ifndef TBC_DEFERREDUSEREFFECT_HPP
#define TBC_DEFERREDUSEREFFECT_HPP

#include <optional>

#include "tbc/Battle.hpp"
#include "tbc/Slot.h"
#include "tbc/Target.h"
#include "tbc/UserEffect.hpp"

namespace ngl::tbc {
template <typename TBattle, typename TEvents, typename TCommands>
class DeferredUserEffect {
  using Result = UserEffect<TBattle, TEvents, TCommands>::Result;

public:
  DeferredUserEffect(Slot::Index user, const UserEffect<TBattle, TEvents, TCommands> &effect, const std::vector<Target> &targets) : user_{user}, effect_{effect}, targets_{targets} {}

  [[nodiscard]] Result Apply(TBattle &b) {
    return effect_.Apply(user_, b, targets_);
  }

protected:
  Slot::Index user_;
  UserEffect<TBattle, TEvents, TCommands> effect_;
  std::vector<Target> targets_;
};
} // namespace ngl::tbc

#endif