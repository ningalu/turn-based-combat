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
template <typename TUnit, typename TBattleState, typename TCommands, typename TCommandResult, typename TEvents>
struct BattleTypes {
  using Commands              = TCommands;
  using CommandResult         = TCommandResult;
  using CommandPayload        = TCommands::Payload;
  using CommandPayloadTypeSet = TCommands::PayloadTypeSet;
  using PlayerComms           = PlayerComms<Commands, CommandResult>;

  using Events       = TEvents;
  using EventPayload = Events::Payload;

  using Battle          = Battle<TUnit, TBattleState, TCommands, TCommandResult>;
  using BattleScheduler = BattleScheduler<TUnit, TBattleState, TCommands, CommandResult, TEvents>;

  using EventHandler = EventHandler<Battle, TCommands, TEvents>;

  using Effect             = Effect<Battle, TEvents, TCommands>;
  using UserEffect         = UserEffect<Battle, TEvents, TCommands>;
  using DeferredEffect     = DeferredEffect<Battle, TEvents, TCommands>;
  using DeferredUserEffect = DeferredUserEffect<Battle, TEvents, TCommands>;
  using EffectResult       = Effect::Result;

  using Action = Action<Battle, TEvents, TCommands>;
};
}; // namespace ngl::tbc

#endif