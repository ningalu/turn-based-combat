#ifndef TBC_USEREFFECT_HPP
#define TBC_USEREFFECT_HPP

#include "tbc/Effect.hpp"
#include "tbc/Slot.h"
#include "tbc/Target.h"

namespace ngl::tbc {
template <typename TBattle>
class UserEffect {
public:
  using Result = Effect<TBattle>::Result;

  UserEffect(std::function<Result(Slot::Index, TBattle &, const std::vector<Target> &)> f) : xfer_{f} {}

  Result Apply(Slot::Index user, TBattle &battle, const std::vector<Target> &targets) {
    return CheckBattle(battle, xfer_(user, battle, targets));
  }

  [[nodiscard]] Result CheckBattle(const TBattle &battle, const Result &initial_result) {
    return outcome_check_ ? outcome_check_(battle, initial_result) : initial_result;
  }

private:
  std::function<Result(Slot::Index, TBattle &, const std::vector<Target> &)> xfer_;
  std::function<Result(const TBattle &, const Result &)> outcome_check_;
};
} // namespace ngl::tbc

#endif