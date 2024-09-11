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

  template <typename TSpecificEvent>
  using Callback = std::function<Action<TBattle>(TSpecificEvent)>;

private:
  template <typename TBattle, typename... Ts>
  struct Detail {
    using Payload  = std::variant<Ts...>;
    using Handlers = std::tuple<Callback<Ts>...>;
  };

  using Events = Detail<TBattle, BattleStart, BattleEnd, TurnsStart, TurnsEnd, ActionStart, ActionEnd, Ts...>;

public:
  using Battle   = TBattle;
  using Payload  = Events::Payload;
  using Handlers = Events::Handlers;

  Payload payload;
};
} // namespace ngl::tbc

#endif