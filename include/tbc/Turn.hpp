#ifndef TBC_TURN_HPP
#define TBC_TURN_HPP

#include <optional>
#include <vector>

#include "tbc/Action.hpp"

namespace ngl::tbc {
template <typename TBattle, typename TEvents, typename TCommands>
struct Turn {
  // TODO: this is functionally equivalent to CommandQueue but with actions. is this important?
  Turn() = default;
  Turn(const std::vector<Action<TBattle, TEvents, TCommands>> &static_actions_) : static_actions{static_actions_} {}

  [[nodiscard]] bool Done() const noexcept {
    return (dynamic_actions.size() == 0) && (static_actions.size() == 0);
  }

  void AddStaticAction(const Action<TBattle, TEvents, TCommands> &action) {
    static_actions.insert(static_actions.begin(), action);
  }

  void AddStaticActions(const std::vector<Action<TBattle, TEvents, TCommands>> &action) {
    static_actions.insert(static_actions.begin(), action.begin(), action.end());
  }

  void AddDynamicAction(const Action<TBattle, TEvents, TCommands> &action) {
    dynamic_actions.insert(dynamic_actions.begin(), action);
  }

  void AddDynamicActions(const std::vector<Action<TBattle, TEvents, TCommands>> &action) {
    dynamic_actions.insert(dynamic_actions.begin(), action.begin(), action.end());
  }

  std::vector<Action<TBattle, TEvents, TCommands>> dynamic_actions, static_actions;
};
} // namespace ngl::tbc

#endif