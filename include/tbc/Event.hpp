#ifndef TBC_EVENT_HPP
#define TBC_EVENT_HPP

#include <functional>
#include <tuple>
#include <variant>
#include <vector>

#include "tbc/Action.hpp"

namespace ngl::tbc {
template <typename TBattle, typename... Ts>
struct Event {
public:
  struct BattleStart {};
  struct BattleEnd {
    std::vector<std::size_t> winners;
  };

  struct TurnsStart {};
  struct TurnsEnd {};

  struct ActionStart {};
  struct ActionEnd {};

private:
  template <typename TBattle, typename... Ts>
  struct CombinedEvents {
    using Payload  = std::variant<Ts...>;
    using Handlers = std::tuple<std::function<Action<TBattle>(Ts)>...>;
  };

  using Events = CombinedEvents<TBattle, BattleStart, BattleEnd, TurnsStart, TurnsEnd, ActionStart, ActionEnd, Ts...>;

public:
  using Payload  = Events::Payload;
  using Handlers = Events::Handlers;

  Payload payload;
};
} // namespace ngl::tbc

#endif