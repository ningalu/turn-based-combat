#ifndef TBC_TURN_HPP
#define TBC_TURN_HPP

#include <optional>
#include <vector>

#include "tbc/Action.hpp"

namespace ngl::tbc {
template <typename TBattle>
struct Turn {
  Turn(const std::vector<Action<TBattle>> &static_actions_) : static_actions{static_actions_} {}

  void AddAction(const Action<TBattle> &action) {
    dynamic_actions.insert(dynamic_actions.begin(), action);
  }

  void AddActions(const std::vector<Action<TBattle>> &action) {
    dynamic_actions.insert(dynamic_actions.begin(), action.begin(), action.end());
  }

  std::vector<Action<TBattle>> dynamic_actions, static_actions;
};
} // namespace ngl::tbc

#endif