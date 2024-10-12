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
template <typename TUnit, typename TBattleState, typename TCommands, typename TEvents>
struct BattleTypes {
  using Commands       = TCommands;
  using CommandPayload = TCommands::Payload;
  using PlayerComms    = PlayerComms<CommandPayload>;

  using Events       = TEvents;
  using EventPayload = Events::Payload;

  using Battle          = Battle<TUnit, TBattleState, TCommands>;
  using BattleScheduler = BattleScheduler<TCommands, Battle, TEvents>;

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