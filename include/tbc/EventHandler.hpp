#ifndef TBC_EVENTHANDLER_HPP
#define TBC_EVENTHANDLER_HPP

#include <cassert>
#include <functional>

#include "tbc/Action.hpp"
#include "tbc/Event.hpp"

namespace ngl::tbc {

namespace detail {
template <typename TBattle, typename TEvents, typename TCommands, typename TEvent>
using EventHandlerCallback = std::function<std::vector<Action<TBattle, TCommands, TEvents>>(TEvent, TBattle &)>;
} // namespace detail

template <typename TBattle, typename TCommands, typename TEvent>
class EventHandler;

template <typename TBattle, typename TCommands, typename... TUserPayloads>
class EventHandler<TBattle, TCommands, Event<TUserPayloads...>> {
public:
  template <typename TSpecificEvent>
  using Callback = detail::EventHandlerCallback<TBattle, Event<TUserPayloads...>, TCommands, TSpecificEvent>;

protected:
  template <typename TEventDetail>
  struct HandlerDetail;

  template <typename... TPayloads>
  struct HandlerDetail<detail::EventDetail<TPayloads...>> {
    using Callbacks = std::tuple<Callback<TPayloads>...>;
  };

  using Callbacks = typename HandlerDetail<typename Event<TUserPayloads...>::Detail>::Callbacks;

public:
  template <typename TSpecificEvent>
  [[nodiscard]] bool HasHandler() const {
    return static_cast<bool>(std::get<Callback<TSpecificEvent>>(callbacks_));
  }

  template <typename TSpecificEvent>
  void RegisterHandler(const Callback<TSpecificEvent> &handler) {
    std::get<Callback<TSpecificEvent>>(callbacks_) = handler;
  }

  template <typename TSpecificEvent>
  void ResetHandler() {
    std::get<Callback<TSpecificEvent>>(callbacks_) = nullptr;
  }

  template <typename TSpecificEvent>
  [[nodiscard]] std::vector<Action<TBattle, TCommands, Event<TUserPayloads...>>> PostEvent(const TSpecificEvent &e, TBattle &b) const {
    assert(HasHandler<TSpecificEvent>());
    return std::get<Callback<TSpecificEvent>>(callbacks_)(e, b);
  }

protected:
  Callbacks callbacks_;
};

} // namespace ngl::tbc

#endif