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
#include "tbc/EventHandler.hpp"

namespace ngl::tbc {
template <typename TCommand, typename TBattle, typename TEvents>
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

  template <typename TSpecificEvent>
  [[nodiscard]] std::optional<Action<TBattle>> PostEvent(const TSpecificEvent &e) const {
    std::optional<Action<TBattle>> out = std::nullopt;
    if (event_handlers_.HasHandler<TSpecificEvent>()) {
      out = event_handlers_.PostEvent<TSpecificEvent>(e);
    }
    return out;
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

  void SetActionTranslator(const std::function<std::vector<Action<TBattle>>(const std::vector<TCommand> &)> &translator) {
    action_translator_ = translator;
  }

  [[nodiscard]] std::vector<Action<TBattle>> GetActions(const std::vector<TCommand> &commands) const {
    assert(action_translator_);
    return action_translator_(commands);
  }

  void ExecuteAction(Action<TBattle> &action, TBattle &b) {
    auto next = action.ApplyNext(b);
    while (next != std::nullopt) {
      auto res = next.value();

      next = action.ApplyNext(b);
    }
  }

  // TODO: to buffer static and dynamic actions
  void RunTurn(std::vector<Action<TBattle>> &actions, TBattle &b) {
    auto pre_turn = PostEvent<TEvents::TurnsStart>({});
    if (pre_turn.has_value()) {
      ExecuteAction(pre_turn.value(), b);
    }

    for (auto &action : actions) {
      ExecuteAction(action, b);
    }

    auto post_turn = PostEvent<TEvents::TurnsEnd>({});
    if (post_turn.has_value()) {
      ExecuteAction(post_turn.value(), b);
    }
  }

protected:
  std::vector<std::unique_ptr<PlayerComms<TCommandPayload>>> players_;

  EventHandler<TEvents> event_handlers_;

  std::function<bool(std::size_t, std::vector<TCommandPayload>)> command_validator_;
  std::function<std::vector<TCommand>(const std::vector<TCommand> &)> command_orderer_;
  std::function<std::vector<Action<TBattle>>(const std::vector<TCommand> &)> action_translator_;
};
} // namespace ngl::tbc

#endif