#ifndef TBC_EFFECT_HPP
#define TBC_EFFECT_HPP

#include <vector>

#include "tbc/Battle.hpp"
#include "tbc/Slot.h"
#include "tbc/Target.h"

namespace ngl::tbc {

namespace EffectResult {

struct Success {};
struct EndBattle {
  std::vector<std::size_t> winners;
};
struct RequestCommands {
  std::vector<std::size_t> players;
};
using Result = std::variant<Success, EndBattle, RequestCommands>;
} // namespace EffectResult

template <typename TBattle>
class Effect {
public:
  using Result = EffectResult::Result;

  Effect(std::function<Result(TBattle &, const std::vector<Target> &)> f) : xfer_{f} {}

  Result Apply(TBattle &battle, const std::vector<Target> &targets) {
    return CheckBattle(battle, xfer_(battle, targets));
  }

  [[nodiscard]] Result CheckBattle(const TBattle &battle, const Result &initial_result) {
    return outcome_check_ ? outcome_check_(battle, initial_result) : initial_result;
  }

private:
  std::function<Result(TBattle &, const std::vector<Target> &)> xfer_;
  std::function<Result(const TBattle &, const Result &)> outcome_check_;
};
} // namespace ngl::tbc

#endif