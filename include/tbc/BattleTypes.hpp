#ifndef TBC_BATTLETYPES_HPP
#define TBC_BATTLETYPES_HPP

#include "tbc/Action.hpp"
#include "tbc/Battle.hpp"
#include "tbc/CommandResponse.hpp"
#include "tbc/Effect.hpp"
#include "tbc/EventHandler.hpp"
#include "tbc/PlayerComms.hpp"
#include "tbc/Schedule.hpp"
#include "tbc/Scheduler.hpp"

namespace ngl::tbc {
template <typename TBattleState_, typename TCommand_, typename TCommandError_, typename TEvent_, SimultaneousActionStrategy TSimultaneousActionStrategy = SimultaneousActionStrategy::DISABLED>
struct BattleTypes {
  using TBattleState  = TBattleState_;
  using TCommand      = TCommand_;
  using TCommandError = TCommandError_;
  using TEvent        = TEvent_;

  using TCommandPayload        = typename TCommand::Payload;
  using TCommandPayloadTypeSet = typename TCommand::PayloadTypeSet;
  using TCommandRequest        = PlayerCommandRequest<TCommand>;
  using TCommandResponse       = CommandResponse<TCommandError>;

  using TEventPayload = typename TEvent::Payload;

  using TBattle      = Battle<TBattleState, TCommand, TCommandError, TEvent, TSimultaneousActionStrategy>;
  using TSchedule    = Schedule<TCommand, TEvent, TSimultaneousActionStrategy>;
  using TScheduler   = Scheduler<TBattleState, TCommand, TCommandError, TEvent, TSimultaneousActionStrategy>;
  using TComms       = Comms<TBattle, TCommand, TCommandError>;
  using TPlayerComms = PlayerComms<TBattle, TCommand, TCommandError>;

  using TEventHandler = EventHandler<TBattle, TCommand, TEvent>;

  using TEffect       = Effect<TBattle, TCommand, TEvent>;
  using TEffectResult = typename TEffect::Result;

  using TAction     = Action<TBattle, TCommand, TEvent>;
  using TActionable = Actionable<TCommand, TEvent, TSimultaneousActionStrategy>;
};
} // namespace ngl::tbc

#endif