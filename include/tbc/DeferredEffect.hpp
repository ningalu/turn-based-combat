#ifndef TBC_DEFERREDEFFECT_HPP
#define TBC_DEFERREDEFFECT_HPP

#include <optional>

#include "tbc/Actionable.hpp"
#include "tbc/Battle.hpp"
#include "tbc/Effect.hpp"
#include "tbc/Target.hpp"

namespace ngl::tbc {
template <typename TBattle, typename TCommands, typename TEvent>
class DeferredEffect {

  using TEffect = Effect<TBattle, TCommands, TEvent>;
  using Result  = typename TEffect::Result;

public:
  DeferredEffect(TEffect effect, std::vector<Target> targets) : effect_{std::move(effect)}, targets_{std::move(targets)} {}

  [[nodiscard]] Result Apply(TBattle &b) {
    return effect_.Apply(b, targets_);
  }

protected:
  TEffect effect_;
  std::vector<Target> targets_;
};
} // namespace ngl::tbc

#endif