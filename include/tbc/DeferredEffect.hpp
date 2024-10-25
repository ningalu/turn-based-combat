#ifndef TBC_DEFERREDEFFECT_HPP
#define TBC_DEFERREDEFFECT_HPP

#include <optional>

#include "tbc/Battle.hpp"
#include "tbc/Effect.hpp"
#include "tbc/Target.hpp"

namespace ngl::tbc {
template <typename TBattle, typename TEvent, typename TCommands>
class DeferredEffect {
  using Result = typename Effect<TBattle, TEvent, TCommands>::Result;

public:
  DeferredEffect(Effect<TBattle, TEvent, TCommands> effect, std::vector<Target> targets) : effect_{std::move(effect)}, targets_{std::move(targets)} {}

  [[nodiscard]] Result Apply(TBattle &b) {
    return effect_.Apply(b, targets_);
  }

protected:
  Effect<TBattle, TEvent, TCommands> effect_;
  std::vector<Target> targets_;
};
} // namespace ngl::tbc

#endif