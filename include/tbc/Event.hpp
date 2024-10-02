#ifndef TBC_EVENT_HPP
#define TBC_EVENT_HPP

#include <variant>

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

namespace detail {
template <typename... TPayloads>
struct EventDetail {
  using Payload = std::variant<TPayloads...>;
};
} // namespace detail

// TODO: decouple from TBattle; find a way to define just the event payload while also getting the Callback tuple into EventHandler
template <typename... TUserPayloads>
struct Event {

  using Detail = detail::EventDetail<
    DefaultEvents::BattleStart,
    DefaultEvents::BattleEnd,
    DefaultEvents::TurnsStart,
    DefaultEvents::TurnsEnd,
    DefaultEvents::ActionStart,
    DefaultEvents::ActionEnd,
    TUserPayloads...>;

  using Payload = typename Detail::Payload;
  Payload payload;
};
} // namespace ngl::tbc

#endif