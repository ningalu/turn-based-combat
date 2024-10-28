#ifndef TBC_EFFECT_HPP
#define TBC_EFFECT_HPP

#include <vector>

#include "tbc/Actionable.hpp"
#include "tbc/Battle.hpp"

namespace ngl::tbc {

enum class EffectStatus {
  SUCCESS,
  SKIPPED,
  FAILED,
  STOP,
};

template <typename TCommand, typename TEvent, SimultaneousActionStrategy TSimultaneousActionStrategy>
struct EffectResult {
  EffectStatus status;
  std::optional<std::vector<std::size_t>> winners;
  std::vector<Actionable<TCommand, TEvent, TSimultaneousActionStrategy>> queued_actions;
};
// TODO: you technically should be able to define your own transfer function shouldnt you
template <typename TBattle, typename TCommand, typename TEvent>
class Effect {
  constexpr static auto TSimultaneousActionStrategy = TBattle::SimultaneousActionStrategy;

public:
  using Result           = EffectResult<TCommand, TEvent, TSimultaneousActionStrategy>;
  using TransferFunction = std::function<Result(TBattle &)>;

  explicit Effect(TransferFunction f) : transfer_function{std::move(f)} {}

  Result Apply(TBattle &battle) {
    assert(transfer_function);
    return transfer_function(battle);
  }

private:
  TransferFunction transfer_function;
};

} // namespace ngl::tbc

#endif