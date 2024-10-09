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
  using TCommandPayload  = TCommand::Payload;
  using TAction          = Action<TBattle, TEvents, TCommand>;
  using CommandValidator = std::function<std::optional<std::vector<TCommandPayload>>(std::size_t, std::vector<TCommandPayload>, const std::vector<TCommand> &, const TBattle &)>;
  using CommandOrderer   = std::function<std::vector<TCommand>(const std::vector<TCommand> &, const TBattle &)>;
  using ActionTranslator = std::function<std::vector<TAction>(const std::vector<TCommand> &, const TBattle &)>;
  using BattleEndChecker = std::function<std::optional<std::vector<std::size_t>>(const TBattle &)>;

public:
  BattleScheduler(std::vector<std::unique_ptr<PlayerComms<TCommandPayload>>> players) : players_{std::move(players)} {}

  [[nodiscard]] std::size_t PlayerCount() const noexcept {
    return players_.size();
  }

  [[nodiscard]] std::vector<TCommand> RequestCommands(const std::vector<std::size_t> &players, const TBattle &battle, std::size_t attempts = 10) {

    std::vector<std::future<std::vector<TCommand>>> action_handles;

    for (const auto player : players) {
      assert(players_.size() > player);

      // Async poll each player for static commands, rejecting and repolling if any are invalid
      action_handles.push_back(std::async(std::launch::async, [=, this]() {
        std::optional<std::vector<TCommandPayload>> payloads;
        for (std::size_t i = 0; i < attempts; i++) {
          const auto incoming_payloads = players_.at(player)->GetStaticCommand().get();
          payloads                     = ValidateCommands(player, incoming_payloads, (queued_commands_.size() > 0 ? queued_commands_.at(0) : std::vector<TCommand>{}), battle);
          if (payloads.has_value()) {
            break;
          }
        }

        if (!payloads.has_value()) {
          return std::vector<TCommand>{};
        }

        std::vector<TCommand> commands;
        for (const auto &payload : payloads.value()) {
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
  void SetHandler(std::function<TAction(TSpecificEvent)> handler) {
    event_handlers_.RegisterHandler<TSpecificEvent>(handler);
  }

  template <typename TSpecificEvent>
  [[nodiscard]] std::optional<TAction> PostEvent(const TSpecificEvent &e) const {
    std::optional<TAction> out = std::nullopt;
    if (event_handlers_.HasHandler<TSpecificEvent>()) {
      out = event_handlers_.PostEvent<TSpecificEvent>(e);
    }
    return out;
  }

  [[nodiscard]] std::optional<TAction> PostEvent(const TEvents &e) const {
    return std::visit([&, this](auto &&event) {
      using T = std::decay_t<decltype(event)>;
      return PostEvent<T>(event);
    },
                      e.payload);
  }

  void SetCommandValidator(const CommandValidator &validator) {
    command_validator_ = validator;
  }

  // TODO: is there actually any meaningful domain-agnostic validation that can be done here?
  [[nodiscard]] std::optional<std::vector<TCommandPayload>> ValidateCommands(std::size_t authority, const std::vector<TCommandPayload> &commands, const std::vector<TCommand> &buffered, const TBattle &battle) const {
    if (command_validator_) {
      return command_validator_(authority, commands, buffered, battle);
    }
    auto out = commands;
    for (const auto &b : buffered) {
      out.push_back(b.payload);
    }
    return commands;
  }

  void SetCommandOrderer(const CommandOrderer &orderer) {
    command_orderer_ = orderer;
  }

  [[nodiscard]] std::vector<TCommand> OrderCommands(const std::vector<TCommand> &commands, const TBattle &battle) const {
    return command_orderer_ ? command_orderer_(commands, battle) : commands;
  }

  void SetActionTranslator(const ActionTranslator &translator) {
    action_translator_ = translator;
  }

  [[nodiscard]] std::vector<TAction> GetActions(const std::vector<TCommand> &commands, const TBattle &battle) const {
    assert(action_translator_);
    return action_translator_(commands, battle);
  }

  void SetBattleEndedChecker(const BattleEndChecker &checker) {
    check_battle_ended_ = checker;
  }

  [[nodiscard]] std::optional<std::vector<std::size_t>> CheckBattleEnded(const TBattle &battle) const {
    assert(check_battle_ended_);
    return check_battle_ended_(battle);
  }

  void RunTurn(Turn<TBattle, TEvents, TCommand> turn, TBattle &b) {
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

    // TODO: temp, figure out how to control post battle effects
    if (!b.HasEnded()) {
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
  }

  [[nodiscard]] std::vector<std::size_t> RunBattle(TBattle &b) {
    const auto player_indices = [&, this]() {
      std::vector<std::size_t> out(players_.size());
      std::iota(out.begin(), out.end(), std::size_t{0});
      return out;
    }();

    for (std::size_t i = 0; !b.HasEnded(); i++) {

      const auto commands         = RequestCommands(player_indices, b);
      const auto ordered_commands = OrderCommands(commands, b);
      const auto actions          = GetActions(ordered_commands, b);
      Turn<TBattle, TEvents, TCommand> turn{actions};
      if (i == 0) {
        auto event_action = PostEvent(DefaultEvents::BattleStart{});
        if (event_action.has_value()) {
          turn.AddAction(event_action.value());
        }
      }
      RunTurn(turn, b);

      if (queued_commands_.size() > 0) {
        queued_commands_.erase(queued_commands_.begin());
      }
    }

    return b.GetWinners();
  }

protected:
  [[nodiscard]] bool RunTurnUntilRestart(Turn<TBattle, TEvents, TCommand> &turn, TBattle &b) {
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

          [[maybe_unused]] auto [status, winners, command_requests, events, buffered_commands] = res.value();

          bool restart = false;

          // battle ending can sometimes still require effects to continue to be applied? TODO: research
          if (winners.winners.size() > 0) {
            b.EndBattle(winners.winners);
            return false;
          }

          const auto manual_winner_check = CheckBattleEnded(b);
          if (manual_winner_check.has_value()) {
            b.EndBattle(manual_winner_check.value());
            return false;
          }

          // TODO: This order causes event actions to run before requested commands. give more control over this?
          // allow effects to trigger these things themselves? that would require a lot of changes
          if (command_requests.players.size() > 0) {
            auto commands        = RequestCommands(command_requests.players, b);
            auto dynamic_actions = GetActions(commands, b);
            turn.AddActions(dynamic_actions);
            restart = true;
          }

          for (auto it = events.events.rbegin(); it != events.events.rend(); it++) {
            TEvents queued_event    = *it;
            const auto event_action = PostEvent(queued_event);
            if (event_action.has_value()) {
              turn.AddAction(event_action.value());
            }
          };

          for (const auto &[command, time] : buffered_commands.commands) {
            BufferCommand(command, time);
          }

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

  void BufferCommand(const TCommand &c, std::size_t turns_ahead) {
    while (queued_commands_.size() < (turns_ahead + 1)) {
      queued_commands_.push_back({});
    }

    queued_commands_.at(turns_ahead).push_back(c);
  }

  std::vector<std::unique_ptr<PlayerComms<TCommandPayload>>> players_;

  EventHandler<TBattle, TCommand, TEvents> event_handlers_;

  CommandValidator command_validator_;
  CommandOrderer command_orderer_;
  ActionTranslator action_translator_;
  BattleEndChecker check_battle_ended_;

  std::vector<std::vector<TCommand>> queued_commands_;
};
} // namespace ngl::tbc

#endif