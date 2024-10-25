#ifndef TBC_EFFECT_HPP
#define TBC_EFFECT_HPP

#include <vector>

#include "tbc/Actionable.hpp"
#include "tbc/Battle.hpp"
#include "tbc/Slot.h"
#include "tbc/Target.hpp"

namespace ngl::tbc {

namespace EffectResult {

enum class Status {
  SUCCESS,
  SKIPPED,
  FAILED,
  STOP,
};

struct Winners {
  std::vector<std::size_t> winners;
};

template <typename TEvent>
struct Events {
  std::vector<TEvent> events;
};

template <typename TCommands, typename TEvent, SimultaneousActionStrategy TSimultaneousActionStrategy>
// TODO: are events actionable? i think so? that would solve the issue of buffered commands/event queue order
using Result = std::tuple<Status, std::optional<std::vector<std::size_t>>, std::vector<Actionable<TCommands, TEvent, TSimultaneousActionStrategy>>>;
} // namespace EffectResult

template <typename TBattle, typename TCommands, typename TEvent>
class Effect {
  constexpr static auto TSimultaneousActionStrategy = TBattle::SimultaneousActionStrategy;

public:
  using Result = EffectResult::Result<TCommands, TEvent, TSimultaneousActionStrategy>;

  explicit Effect(std::function<Result(TBattle &, const std::vector<Target> &)> f) : xfer_{std::move(f)} {}

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