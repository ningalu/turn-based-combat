#ifndef TBC_TURN_HPP
#define TBC_TURN_HPP

#include <optional>
#include <vector>

#include "tbc/Action.hpp"

namespace ngl::tbc {
template <typename TBattle, typename TEvents, typename TCommands>
struct Turn {
  Turn(const std::vector<Action<TBattle, TEvents, TCommands>> &static_actions_) : static_actions{static_actions_} {}

  void AddAction(const Action<TBattle, TEvents, TCommands> &action) {
    dynamic_actions.insert(dynamic_actions.begin(), action);
  }

  void AddActions(const std::vector<Action<TBattle, TEvents, TCommands>> &action) {
    dynamic_actions.insert(dynamic_actions.begin(), action.begin(), action.end());
  }

  std::vector<Action<TBattle, TEvents, TCommands>> dynamic_actions, static_actions;
};
} // namespace ngl::tbc

#endif