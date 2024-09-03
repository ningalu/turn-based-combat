#ifndef TBC_BATTLESCHEDULER_HPP
#define TBC_BATTLESCHEDULER_HPP

#include <cassert>
#include <functional>
#include <future>
#include <vector>

#include "Battle.hpp"
#include "Command.hpp"

namespace ngl::tbc {
template <typename TCommand, typename TBattle>
class BattleScheduler {
public:
  BattleScheduler(std::vector<std::unique_ptr<PlayerComms<TCommand>>> players) : players_{std::move(players)} {}

  [[nodiscard]] std::vector<TCommand> RequestCommands(const std::vector<std::size_t> &players) {
    std::vector<std::future<std::vector<TCommand>>> action_handles;

    for (const auto p : players) {
      assert(players_.size() > p);

      // Async poll each player for static commands, rejecting and repolling if any are invalid
      action_handles.push_back(std::async(std::launch::async, [=, this]() {
        std::vector<TCommand> s;
        do {
          s = players_.at(p)->GetStaticCommand().get();
        } while (!ValidateCommands(p, s));

        return s;
      }));
    }

    std::vector<TCommand> actions;
    for (auto &response : action_handles) {
      auto val = response.get();
      actions.insert(actions.end(), val.begin(), val.end());
    }
    return actions;
  }

  void SetCommandValidator(const std::function<bool(std::size_t, std::vector<TCommand>)> &validator) {
    command_validator_ = validator;
  }

  // TODO: is there actually any meaningful domain-agnostic validation that can be done here?
  [[nodiscard]] bool ValidateCommands(std::size_t authority, const std::vector<TCommand> &commands) const {
    if (command_validator_) {
      if (!command_validator_(authority, commands)) {
        return false;
      }
    }
    return true;
  }

  void SetCommandOrderer(const std::function<std::vector<TCommand>(const std::vector<TCommand> &)> &orderer) {
    command_orderer_ = orderer;
  }

  [[nodiscard]] std::vector<TCommand> OrderCommands(const std::vector<TCommand> &commands) const {
    assert(command_orderer_);
    return command_orderer_(commands);
  }

protected:
  std::vector<std::unique_ptr<PlayerComms<TCommand>>> players_;

  std::function<bool(std::size_t, std::vector<TCommand>)> command_validator_;
  std::function<std::vector<TCommand>(const std::vector<TCommand> &)> command_orderer_;
};
} // namespace ngl::tbc

#endif