#ifndef TBC_BATTLESCHEDULER_HPP
#define TBC_BATTLESCHEDULER_HPP

#include <cassert>
#include <functional>
#include <future>
#include <optional>
#include <vector>

#include "tbc/Action.hpp"
#include "tbc/Battle.hpp"
#include "tbc/Command.hpp"

namespace ngl::tbc {
template <typename TCommand, typename TBattle>
class BattleScheduler {
  using TCommandPayload = TCommand::Payload;

public:
  BattleScheduler(std::vector<std::unique_ptr<PlayerComms<TCommandPayload>>> players) : players_{std::move(players)} {}

  [[nodiscard]] std::vector<TCommand> RequestCommands(const std::vector<std::size_t> &players) {
    std::vector<std::future<std::vector<TCommand>>> action_handles;

    for (const auto player : players) {
      assert(players_.size() > player);

      // Async poll each player for static commands, rejecting and repolling if any are invalid
      action_handles.push_back(std::async(std::launch::async, [=, this]() {
        std::vector<TCommandPayload> payloads;
        do {
          payloads = players_.at(player)->GetStaticCommand().get();

        } while (!ValidateCommands(player, payloads));

        std::vector<TCommand> commands;
        for (const auto &payload : payloads) {
          commands.push_back(TCommand{player, payload});
        }
        return commands;
      }));
    }

    std::vector<TCommand> actions;
    for (auto &response : action_handles) {
      auto val = response.get();
      actions.insert(actions.end(), val.begin(), val.end());
    }
    return actions;
  }

  void SetCommandValidator(const std::function<bool(std::size_t, std::vector<TCommandPayload>)> &validator) {
    command_validator_ = validator;
  }

  // TODO: is there actually any meaningful domain-agnostic validation that can be done here?
  [[nodiscard]] bool ValidateCommands(std::size_t authority, const std::vector<TCommandPayload> &commands) const {
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
    return command_orderer_ ? command_orderer_(commands) : commands;
  }

  void SetActionTranslator(const std::function<std::vector<Action<UserEffect<TBattle>>>(const std::vector<TCommand> &)> &translator) {
    action_translator_ = translator;
  }

  [[nodiscard]] std::vector<Action<UserEffect<TBattle>>> GetActions(const std::vector<TCommand> &commands) const {
    assert(action_translator_);
    return action_translator_(commands);
  }

  void ExecuteAction(const Action<UserEffect<TBattle>> &action, TBattle &b) {
    auto next = action.NextEffect();
    while (next != std::nullopt) {
      auto effect       = next.value();
      const auto result = effect.Apply(action.user, b, action.targets);

      next = action.NextEffect();
    }
  }

protected:
  std::vector<std::unique_ptr<PlayerComms<TCommandPayload>>> players_;

  std::function<bool(std::size_t, std::vector<TCommandPayload>)> command_validator_;
  std::function<std::vector<TCommand>(const std::vector<TCommand> &)> command_orderer_;
  std::function<std::vector<Action<UserEffect<TBattle>>>(const std::vector<TCommand> &)> action_translator_;
};
} // namespace ngl::tbc

#endif