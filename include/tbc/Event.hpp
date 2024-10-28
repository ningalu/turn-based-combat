#ifndef TBC_EVENT_HPP
#define TBC_EVENT_HPP

#include <variant>

namespace ngl::tbc {
namespace DefaultEvents {

struct BattleStart {};
struct BattleEnd {
  std::vector<std::size_t> winners;
};

struct TurnStart {};
struct TurnEnd {};

struct PlannedActionStart {};
struct PlannedActionEnd {};
} // namespace DefaultEvents

namespace detail {
template <typename... TPayloads>
struct EventDetail {
  using Payload = std::variant<TPayloads...>;
};
} // namespace detail

template <typename... TUserPayloads>
struct Event {

  using Detail = detail::EventDetail<
    DefaultEvents::BattleStart,
    DefaultEvents::BattleEnd,
    DefaultEvents::TurnStart,
    DefaultEvents::TurnEnd,
    DefaultEvents::PlannedActionStart,
    DefaultEvents::PlannedActionEnd,
    TUserPayloads...>;

  using Payload = typename Detail::Payload;
  Payload payload;
};
} // namespace ngl::tbc

#endif