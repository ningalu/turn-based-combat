#ifndef TBC_DEFERREDEFFECT_HPP
#define TBC_DEFERREDEFFECT_HPP

#include <optional>

#include "tbc/Battle.hpp"
#include "tbc/Effect.hpp"
#include "tbc/Target.h"

namespace ngl::tbc {
template <typename TBattle>
class DeferredEffect {
  using Result = Effect<TBattle>::Result;

public:
  DeferredEffect(const std::vector<Effect<TBattle>> &effects, const std::vector<Target> &targets) : effects_{effects}, targets_{targets} {}
  [[nodiscard]] std::optional<Result> ApplyNext(TBattle &b) {
    std::optional<Result> out = std::nullopt;
    if (effects_.size() > 0) {
      out = effects_.at(0).Apply(b, targets_);
      effects_.erase(effects_.begin());
    }
    return out;
  }

  [[nodiscard]] bool Done() const {
    return !(effects_.size() > 0);
  }

protected:
  std::vector<Effect<TBattle>> effects_;
  std::vector<Target> targets_;
};
} // namespace ngl::tbc

#endif