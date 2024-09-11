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
  using EventHandler = EventHandler<TEvents>;

  template <typename TSpecificEvent>
  using EventCallback = Events::Callback;

  using Battle          = Battle<TUnit, TBattleState>;
  using BattleScheduler = BattleScheduler<TCommands, Battle, TEvents>;

  using Effect             = Effect<Battle>;
  using UserEffect         = UserEffect<Battle>;
  using DeferredEffect     = DeferredEffect<Battle>;
  using DeferredUserEffect = DeferredUserEffect<Battle>;
  using EffectResult       = Effect::Result;

  using Action = Action<Battle>;
};
}; // namespace ngl::tbc

#endif