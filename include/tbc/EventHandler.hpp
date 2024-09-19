#ifndef TBC_EVENTHANDLER_HPP
#define TBC_EVENTHANDLER_HPP

#include <cassert>
#include <functional>

#include "tbc/Action.hpp"
#include "tbc/Event.hpp"

namespace ngl::tbc {

namespace detail {
template <typename TBattle, typename TEvents, typename TCommands, typename TEvent>
using EventHandlerCallback = std::function<Action<TBattle, TEvents, TCommands>(TEvent)>;
}

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

  using Callbacks = HandlerDetail<typename Event<TUserPayloads...>::Detail>::Callbacks;

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
  [[nodiscard]] Action<TBattle, Event<TUserPayloads...>, TCommands> PostEvent(const TSpecificEvent &e) const {
    assert(HasHandler<TSpecificEvent>());
    return std::get<Callback<TSpecificEvent>>(callbacks_)(e);
  }

protected:
  Callbacks callbacks_;
};

} // namespace ngl::tbc

#endif