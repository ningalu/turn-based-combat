#ifndef TBC_BATTLETYPES_HPP
#define TBC_BATTLETYPES_HPP

#include "tbc/Action.hpp"
#include "tbc/Battle.hpp"
#include "tbc/BattleScheduler.hpp"
#include "tbc/Effect.hpp"
#include "tbc/EventHandler.hpp"
#include "tbc/PlayerComms.hpp"
#include "tbc/Schedule.hpp"

namespace ngl::tbc {
template <typename TBattleState_, typename TCommand_, typename TCommandResult_, typename TEvent_, SimultaneousActionStrategy TSimultaneousActionStrategy = SimultaneousActionStrategy::DISABLED>
struct BattleTypes {
  using TBattleState   = TBattleState_;
  using TCommand       = TCommand_;
  using TCommandResult = TCommandResult_;
  using TEvent         = TEvent_;

  using TCommandPayload        = typename TCommand::Payload;
  using TCommandPayloadTypeSet = typename TCommand::PayloadTypeSet;
  using TCommandRequest        = PlayerCommandRequest<TCommand>;

  using TEventPayload = typename TEvent::Payload;

  using TBattle          = Battle<TBattleState, TCommand, TCommandResult, TEvent, TSimultaneousActionStrategy>;
  using TSchedule        = Schedule<TCommand, TEvent, TSimultaneousActionStrategy>;
  using TBattleScheduler = BattleScheduler<TBattleState, TCommand, TCommandResult, TEvent, TSimultaneousActionStrategy>;
  using TComms           = Comms<TBattle, TCommand, TCommandResult>;
  using TPlayerComms     = PlayerComms<TBattle, TCommand, TCommandResult>;

  using TEventHandler = EventHandler<TBattle, TCommand, TEvent>;

  using TEffect       = Effect<TBattle, TCommand, TEvent>;
  using TEffectResult = typename TEffect::Result;

  using TAction     = Action<TBattle, TCommand, TEvent>;
  using TActionable = Actionable<TCommand, TEvent, TSimultaneousActionStrategy>;
};
} // namespace ngl::tbc

#endif