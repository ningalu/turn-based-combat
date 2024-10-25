#ifndef TBC_DEFERREDUSEREFFECT_HPP
#define TBC_DEFERREDUSEREFFECT_HPP

#include <optional>

#include "tbc/Battle.hpp"
#include "tbc/Slot.h"
#include "tbc/Target.hpp"
#include "tbc/UserEffect.hpp"

namespace ngl::tbc {
template <typename TBattle, typename TCommands, typename TEvent>
class DeferredUserEffect {
  using TUserEffect = UserEffect<TBattle, TCommands, TEvent>;
  using Result      = typename TUserEffect::Result;

public:
  DeferredUserEffect(Slot::Index user, TUserEffect effect, std::vector<Target> targets) : user_{user}, effect_{std::move(effect)}, targets_{std::move(targets)} {}

  [[nodiscard]] Result Apply(TBattle &b) {
    return effect_.Apply(user_, b, targets_);
  }

protected:
  Slot::Index user_;
  TUserEffect effect_;
  std::vector<Target> targets_;
};
} // namespace ngl::tbc

#endif