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
template <typename TUnit_, typename TBattleState_, typename TCommands_, typename TCommandResult_, typename TEvents_>
struct BattleTypes {
  using TUnit          = TUnit_;
  using TBattleState   = TBattleState_;
  using TCommands      = TCommands_;
  using TCommandResult = TCommandResult_;
  using TEvents        = TEvents_;

  using TCommandPayload        = typename TCommands::Payload;
  using TCommandPayloadTypeSet = typename TCommands::PayloadTypeSet;
  using TPlayerComms           = PlayerComms<TCommands, TCommandResult>;

  using TEventPayload = typename TEvents::Payload;

  using TBattle          = Battle<TBattleState, TCommands, TCommandResult>;
  using TBattleScheduler = BattleScheduler<TBattleState, TCommands, TCommandResult, TEvents>;

  using TEventHandler = EventHandler<TBattle, TCommands, TEvents>;

  using TEffect             = Effect<TBattle, TEvents, TCommands>;
  using TUserEffect         = UserEffect<TBattle, TEvents, TCommands>;
  using TDeferredEffect     = DeferredEffect<TBattle, TEvents, TCommands>;
  using TDeferredUserEffect = DeferredUserEffect<TBattle, TEvents, TCommands>;
  using TEffectResult       = typename TEffect::Result;

  using TAction = Action<TBattle, TEvents, TCommands>;
};
} // namespace ngl::tbc

#endif