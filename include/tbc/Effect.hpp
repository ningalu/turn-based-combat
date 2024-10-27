#ifndef TBC_EFFECT_HPP
#define TBC_EFFECT_HPP

#include <vector>

#include "tbc/Actionable.hpp"
#include "tbc/Battle.hpp"
#include "tbc/Slot.h"
#include "tbc/Target.hpp"

namespace ngl::tbc {

enum class EffectStatus {
  SUCCESS,
  SKIPPED,
  FAILED,
  STOP,
};

template <typename TCommands, typename TEvent, SimultaneousActionStrategy TSimultaneousActionStrategy>
using EffectResult = std::tuple<EffectStatus, std::optional<std::vector<std::size_t>>, std::vector<Actionable<TCommands, TEvent, TSimultaneousActionStrategy>>>;

// TODO: you technically should be able to define your own transfer function shouldnt you
template <typename TBattle, typename TCommands, typename TEvent>
class Effect {
  constexpr static auto TSimultaneousActionStrategy = TBattle::SimultaneousActionStrategy;

public:
  using Result           = EffectResult<TCommands, TEvent, TSimultaneousActionStrategy>;
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