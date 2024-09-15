#ifndef TBC_EVENT_HPP
#define TBC_EVENT_HPP

#include <functional>
#include <tuple>
#include <variant>
#include <vector>

#include "tbc/Action.hpp"

namespace ngl::tbc {
namespace DefaultEvents {

struct BattleStart {};
struct BattleEnd {
  std::vector<std::size_t> winners;
};

struct TurnsStart {};
struct TurnsEnd {};

struct ActionStart {};
struct ActionEnd {};
} // namespace DefaultEvents

// TODO: decouple from TBattle; find a way to define just the event payload while also getting the Callback tuple into EventHandler
template <typename TBattle, typename... Ts>
struct Event {
public:
  template <typename TSpecificEvent>
  using Callback = std::function<Action<TBattle>(TSpecificEvent)>;

private:
  template <typename TBattle, typename... Ts>
  struct Detail {
    using Payload  = std::variant<Ts...>;
    using Handlers = std::tuple<Callback<Ts>...>;
  };

  using Events = Detail<
    TBattle,
    DefaultEvents::BattleStart,
    DefaultEvents::BattleEnd,
    DefaultEvents::TurnsStart,
    DefaultEvents::TurnsEnd,
    DefaultEvents::ActionStart,
    DefaultEvents::ActionEnd,
    Ts...>;

public:
  using Battle   = TBattle;
  using Payload  = Events::Payload;
  using Handlers = Events::Handlers;

  Payload payload;
};
} // namespace ngl::tbc

#endif