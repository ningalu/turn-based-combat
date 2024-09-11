#ifndef TBC_EVENTHANDLER_HPP
#define TBC_EVENTHANDLER_HPP

#include <cassert>
#include <functional>

#include "tbc/Action.hpp"
#include "tbc/Event.hpp"

namespace ngl::tbc {
template <typename TEvents>
class EventHandler {
  using Battle = TEvents::Battle;

public:
  template <typename TSpecificEvent>
  [[nodiscard]] bool HasHandler() const {
    return static_cast<bool>(std::get<TEvents::Callback<TSpecificEvent>>(callbacks_));
  }

  // Why doesn't TEvents::Callback<TSpecificEvent> work as the function argument?
  template <typename TSpecificEvent>
  void RegisterHandler(std::function<Action<Battle>(TSpecificEvent)> handler) {
    std::get<TEvents::Callback<TSpecificEvent>>(callbacks_) = handler;
  }

  template <typename TSpecificEvent>
  void ResetHandler() {
    std::get<TEvents::Callback<TSpecificEvent>>(callbacks_) = nullptr;
  }

  template <typename TSpecificEvent>
  [[nodiscard]] Action<Battle> PostEvent(const TSpecificEvent &e) const {
    assert(HasHandler<TSpecificEvent>());
    return std::get<TEvents::Callback<TSpecificEvent>>(callbacks_)(e);
  }

protected:
  TEvents::Handlers callbacks_;
};
} // namespace ngl::tbc

#endif