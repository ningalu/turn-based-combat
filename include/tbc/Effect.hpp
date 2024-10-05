#ifndef TBC_EFFECT_HPP
#define TBC_EFFECT_HPP

#include <vector>

#include "tbc/Battle.hpp"
#include "tbc/Slot.h"
#include "tbc/Target.hpp"

namespace ngl::tbc {

namespace EffectResult {

enum class Status {
  SUCCESS,
  SKIPPED,
  FAILED,
};

struct Winners {
  std::vector<std::size_t> winners;
};

struct RequestCommands {
  std::vector<std::size_t> players;
};

template <typename TEvents>
struct Events {
  std::vector<TEvents> events;
};

template <typename TCommand>
struct BufferCommands {
  std::vector<std::pair<TCommand, std::size_t>> commands;
};

template <typename TEvents, typename TCommands>
using Result = std::tuple<Status, Winners, RequestCommands, Events<TEvents>, BufferCommands<TCommands>>;
} // namespace EffectResult

template <typename TBattle, typename TEvents, typename TCommands>
class Effect {
public:
  using Result = EffectResult::Result<TEvents, TCommands>;

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