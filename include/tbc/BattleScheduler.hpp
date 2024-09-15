#ifndef TBC_BATTLESCHEDULER_HPP
#define TBC_BATTLESCHEDULER_HPP

#include <cassert>
#include <functional>
#include <future>
#include <iostream>
#include <numeric>
#include <optional>
#include <vector>

#include "tbc/Action.hpp"
#include "tbc/Battle.hpp"
#include "tbc/Command.hpp"
#include "tbc/Effect.hpp"
#include "tbc/EventHandler.hpp"
#include "tbc/Turn.hpp"

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
  void SetHandler(std::function<Action<TBattle>(TSpecificEvent)> handler) {
    event_handlers_.RegisterHandler<TSpecificEvent>(handler);
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

  void RunTurn(Turn<TBattle> turn, TBattle &b) {
    auto pre_turn = PostEvent<DefaultEvents::TurnsStart>({});
    if (pre_turn.has_value()) {
      turn.AddAction(pre_turn.value());
    }

    // Run from the start every time a restart is required
    while (RunTurnUntilRestart(turn, b)) {
      if (b.HasEnded()) {
        return;
      }
    }

    auto post_turn = PostEvent<DefaultEvents::TurnsEnd>({});
    if (post_turn.has_value()) {
      turn.AddAction(post_turn.value());
    }

    // Run all actions added by post turn event
    while (RunTurnUntilRestart(turn, b)) {
      if (b.HasEnded()) {
        return;
      }
    }
  }

  [[nodiscard]] std::vector<std::size_t> RunBattle(TBattle &b) {
    const auto player_indices = [&, this]() {
      std::vector<std::size_t> out(players_.size());
      std::iota(out.begin(), out.end(), std::size_t{0});
      return out;
    }();

    for (std::size_t i = 0; !b.HasEnded(); i++) {

      const auto commands         = RequestCommands(player_indices);
      const auto ordered_commands = OrderCommands(commands);
      const auto actions          = GetActions(ordered_commands);
      Turn<TBattle> turn{actions};
      if (i == 0) {
        auto event_action = PostEvent(DefaultEvents::BattleStart{});
        if (event_action.has_value()) {
          turn.AddAction(event_action.value());
        }
      }
      RunTurn(turn, b);
    }

    return b.GetWinners();
  }

protected:
  [[nodiscard]] bool RunTurnUntilRestart(Turn<TBattle> &turn, TBattle &b) {
    for (auto *actions_ptr : {&turn.dynamic_actions, &turn.static_actions}) {
      auto &actions            = *actions_ptr;
      bool trigger_turn_events = (actions_ptr == &turn.static_actions);

      while (actions.size() > 0) {
        auto &action = actions.at(0);

        if (trigger_turn_events && (!action.Started())) {
          std::cout << "Static action start event\n";
        }

        while (!action.Done()) {
          auto res = action.ApplyNext(b);

          if (res == std::nullopt) {
            break;
          }

          auto restart = std::visit([&, this](auto &&r) {
            using T = std::decay_t<decltype(r)>;
            if constexpr (std::is_same_v<T, EffectResult::EndBattle>) {
              // The battle ending doesn't cause the action order to restart, but will stop any additional effects from being applied
              // end the battle
              b.EndBattle(r.winners);
              return false;
            } else if constexpr (std::is_same_v<T, EffectResult::RequestCommands>) {
              // Requesting additional dynamic commands requires working from the start of the action order
              auto commands = RequestCommands(r.players);
              auto actions  = GetActions(commands);
              turn.AddActions(actions);
              return true;
            } else if constexpr (std::is_same_v<T, EffectResult::Success>) {
              return false;
            }
          },
                                    res.value());

          if (restart) {
            return restart;
          }
        }

        if (trigger_turn_events) {
          std::cout << "Static action end events\n";
        }

        actions.erase(actions.begin());
      }
    }

    return false;
  }

  std::vector<std::unique_ptr<PlayerComms<TCommandPayload>>> players_;

  EventHandler<TEvents> event_handlers_;

  std::function<bool(std::size_t, std::vector<TCommandPayload>)> command_validator_;
  std::function<std::vector<TCommand>(const std::vector<TCommand> &)> command_orderer_;
  std::function<std::vector<Action<TBattle>>(const std::vector<TCommand> &)> action_translator_;
};
} // namespace ngl::tbc

#endif