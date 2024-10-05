#ifndef TBC_DEFERREDEFFECT_HPP
#define TBC_DEFERREDEFFECT_HPP

#include <optional>

#include "tbc/Battle.hpp"
#include "tbc/Effect.hpp"
#include "tbc/Target.hpp"

namespace ngl::tbc {
template <typename TBattle, typename TEvents, typename TCommands>
class DeferredEffect {
  using Result = Effect<TBattle, TEvents, TCommands>::Result;

public:
  DeferredEffect(const Effect<TBattle, TEvents, TCommands> &effect, const std::vector<Target> &targets) : effect_{effect}, targets_{targets} {}

  [[nodiscard]] Result Apply(TBattle &b) {
    return effect_.Apply(b, targets_);
  }

protected:
  Effect<TBattle, TEvents, TCommands> effect_;
  std::vector<Target> targets_;
};
} // namespace ngl::tbc

#endif