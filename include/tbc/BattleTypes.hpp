#ifndef TBC_BATTLETYPES_HPP
#define TBC_BATTLETYPES_HPP

#include "tbc/Action.hpp"
#include "tbc/Battle.hpp"
#include "tbc/BattleScheduler.hpp"
#include "tbc/DeferredEffect.hpp"
#include "tbc/DeferredUserEffect.hpp"
#include "tbc/Effect.hpp"
#include "tbc/EventHandler.hpp"
#include "tbc/PlayerComms.hpp"
#include "tbc/UserEffect.hpp"

namespace ngl::tbc {
template <typename TUnit_, typename TBattleState_, typename TCommand_, typename TCommandResult_, typename TEvent_>
struct BattleTypes {
  using TUnit          = TUnit_;
  using TBattleState   = TBattleState_;
  using TCommand       = TCommand_;
  using TCommandResult = TCommandResult_;
  using TEvent         = TEvent_;

  using TCommandPayload        = typename TCommand::Payload;
  using TCommandPayloadTypeSet = typename TCommand::PayloadTypeSet;
  using TPlayerComms           = PlayerComms<TCommand, TCommandResult>;
  using TCommandRequest        = PlayerCommandRequest<TCommand>;

  using TEventPayload = typename TEvent::Payload;

  using TBattle          = Battle<TBattleState, TCommand, TCommandResult, TEvent_>;
  using TSchedule        = typename TBattle::Schedule;
  using TBattleScheduler = BattleScheduler<TBattleState, TCommand, TCommandResult, TEvent>;

  using TEventHandler = EventHandler<TBattle, TCommand, TEvent>;

  using TEffect             = Effect<TBattle, TEvent, TCommand>;
  using TUserEffect         = UserEffect<TBattle, TEvent, TCommand>;
  using TDeferredEffect     = DeferredEffect<TBattle, TEvent, TCommand>;
  using TDeferredUserEffect = DeferredUserEffect<TBattle, TEvent, TCommand>;
  using TEffectResult       = typename TEffect::Result;

  using TAction = Action<TBattle, TEvent, TCommand>;
};
} // namespace ngl::tbc

#endif