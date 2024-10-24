#ifndef TBC_BATTLESCHEDULER_HPP
#define TBC_BATTLESCHEDULER_HPP

#include <cassert>
#include <functional>
#include <future>
#include <iostream>
#include <numeric>
#include <optional>
#include <vector>

#include "util/tmp.hpp"

#include "tbc/Action.hpp"
#include "tbc/Battle.hpp"
#include "tbc/Command.hpp"
#include "tbc/Effect.hpp"
#include "tbc/EventHandler.hpp"
#include "tbc/PlayerCommandRequest.hpp"
#include "tbc/Turn.hpp"

namespace ngl::tbc {

template <typename TCommand>
struct ScheduledCommands {
  std::vector<PlayerCommandRequest<TCommand>> players;
};

struct ImmediateCommands {
  std::vector<std::size_t> players;
};

template <typename TState, typename TCommand, typename TCommandResult, typename TEvents, typename TCommandRequestStrategy>
  requires(tmp::is_present_v<TCommandRequestStrategy, ScheduledCommands<TCommand>, ImmediateCommands>)
class BattleScheduler {
  using TCommandPayload = typename TCommand::Payload;
  using TBattle         = Battle<TState, TCommand, TCommandResult>;
  using TAction         = Action<TBattle, TEvents, TCommand>;
  using TTurn           = Turn<TBattle, TEvents, TCommand>;

  using TActionTranslator = std::function<TAction(const TCommand &, const TBattle &)>;
  using TBattleEndChecker = std::function<std::optional<std::vector<std::size_t>>(const TBattle &)>;

  using TCommandRequestStrategyHandler = std::function<TCommandRequestStrategy(const TBattle &)>;

public:
  TCommandRequestStrategyHandler CommandRequestStrategyHandler;

  template <typename TSpecificEvent>
  void SetHandler(std::function<std::vector<TAction>(TSpecificEvent, TBattle &)> handler) {
    event_handlers_.template RegisterHandler<TSpecificEvent>(handler);
  }

  template <typename TSpecificEvent>
  [[nodiscard]] std::optional<std::vector<TAction>> PostEvent(const TSpecificEvent &e, TBattle &b) const {
    std::optional<std::vector<TAction>> out = std::nullopt;
    // .template looks gross as fuck
    if (event_handlers_.template HasHandler<TSpecificEvent>()) {
      out = event_handlers_.template PostEvent<TSpecificEvent>(e, b);
    }
    return out;
  }

  [[nodiscard]] std::optional<std::vector<TAction>> PostEvent(const TEvents &event, TBattle &battle) const {
    return std::visit([&](auto &&event_payload) {
      using T = std::decay_t<decltype(event_payload)>;
      return PostEvent<T>(event_payload, battle);
    },
                      event.payload);
  }

  void SetActionTranslator(const TActionTranslator &translator) {
    action_translator_ = translator;
  }

  [[nodiscard]] TAction TranslateAction(const TCommand &command, const TBattle &battle) const {
    assert(action_translator_);
    return action_translator_(command, battle);
  }

  [[nodiscard]] std::vector<TAction> TranslateActions(const std::vector<TCommand> &commands, const TBattle &battle) const {
    assert(action_translator_);
    std::vector<TAction> out;
    out.reserve(commands.size());
    for (const auto &command : commands) {
      out.push_back(action_translator_(command, battle));
    }
    return out;
  }

  void SetBattleEndedChecker(const TBattleEndChecker &checker) {
    check_battle_ended_ = checker;
  }

  [[nodiscard]] std::optional<std::vector<std::size_t>> CheckBattleEnded(const TBattle &battle) const {
    assert(check_battle_ended_);
    return check_battle_ended_(battle);
  }

  void RunTurn(Turn<TBattle, TEvents, TCommand> turn, TBattle &b) {

    auto pre_turn = PostEvent<DefaultEvents::TurnsStart>({}, b);
    if (pre_turn.has_value()) {
      turn.AddDynamicActions(pre_turn.value());
    }

    // Run from the start every time a restart is required
    while (RunTurnUntilInterrupted(turn, b)) {
      if (b.HasEnded()) {
        return;
      }
    }

    // TODO: temp, figure out how to control post battle effects
    if (!b.HasEnded()) {
      auto post_turn = PostEvent<DefaultEvents::TurnsEnd>({}, b);
      if (post_turn.has_value()) {
        turn.AddDynamicActions(post_turn.value());
      }

      // Run all actions added by post turn event
      while (RunTurnUntilInterrupted(turn, b)) {
        if (b.HasEnded()) {
          return;
        }
      }
    }
  }

  [[nodiscard]] std::vector<std::size_t> RunBattle(TBattle &battle) {
    for (std::size_t i = 0; !battle.HasEnded(); i++) {
      std::cout << "Start turn " << i + 1 << "\n";

      if constexpr (std::is_same_v<TCommandRequestStrategy, ScheduledCommands<TCommand>>) {

        std::vector<PlayerCommandRequest<TCommand>> requests;
        if (CommandRequestStrategyHandler) {
          requests = CommandRequestStrategyHandler(battle).players;
        } else {
          for (std::size_t player = 0; player < battle.PlayerCount(); player++) {
            requests.push_back(
              PlayerCommandRequest<TCommand>{player, battle.GetValidTurnStartCommands(player)}
            );
          }
        }

        battle.StartTurn(requests);
        battle.current_turn_commands.static_commands = battle.OrderCommands(battle.current_turn_commands.static_commands);
      } else {
        if (CommandRequestStrategyHandler) {
          remaining_immediate_commands_ = CommandRequestStrategyHandler(battle).players;
        } else {
          for (std::size_t player = 0; player < battle.PlayerCount(); player++) {
            remaining_immediate_commands_.push_back(i);
          }
        }
        battle.StartTurn();
      }

      Turn<TBattle, TEvents, TCommand> turn;
      if (i == 0) {
        auto event_action = PostEvent(DefaultEvents::BattleStart{}, battle);
        if (event_action.has_value()) {
          turn.AddDynamicActions(event_action.value());
        }
      }
      RunTurn(turn, battle);
    }

    return battle.GetWinners();
  }

protected:
  [[nodiscard]] bool ApplyActionUntilInterrupted(TAction &action, TTurn &turn, TBattle &battle) {
    const auto timeout = 100;
    for (std::size_t i = 0; i < timeout; i++) {

      const auto result = action.ApplyNext(battle);

      if (!result.has_value()) {
        // Action ran to completion
        return false;
      }

      [[maybe_unused]] auto &[status, winners, commands, events] = result.value();

      if (status == EffectResult::Status::STOP) {
        action.Cancel();
      }

      // if the effect caused the battle to end, report an interruption immediately
      // battle ending can sometimes still require effects to continue to be applied? TODO: research
      if (!winners.winners.empty()) {
        battle.EndBattle(winners.winners);
        return false;
      }

      const auto manual_winner_check = CheckBattleEnded(battle);
      if (manual_winner_check.has_value()) {
        battle.EndBattle(manual_winner_check.value());
        return false;
      }

      std::vector<TAction> new_dynamic_actions = TranslateActions(commands, battle);
      for (const auto &event : events.events) {
        const auto event_action = PostEvent(event, battle);
        if (event_action.has_value()) {
          const auto &event_action_values = event_action.value();
          new_dynamic_actions.insert(new_dynamic_actions.begin(), event_action_values.begin(), event_action_values.end());
        }
      }

      if (!new_dynamic_actions.empty()) {
        turn.AddDynamicActions(new_dynamic_actions);
        return true;
      }
    }
    return false;
  }

  [[nodiscard]] bool RunTurnUntilInterrupted(TTurn &turn, TBattle &battle) {
    auto &queued_commands = battle.current_turn_commands;

    while (!turn.dynamic_actions.empty()) {
      while (ApplyActionUntilInterrupted(turn.dynamic_actions.at(0), turn, battle)) {
        if (battle.HasEnded()) {
          return true;
        }
      }
      turn.dynamic_actions.erase(turn.dynamic_actions.begin());
    }

    if (!queued_commands.dynamic_commands.empty()) {
      const auto queued_dynamic_command = queued_commands.dynamic_commands.at(0);
      queued_commands.dynamic_commands.erase(queued_commands.dynamic_commands.begin());

      auto queued_dynamic_action = TranslateAction(queued_dynamic_command, battle);
      turn.AddDynamicAction(queued_dynamic_action);
      return true;
    }

    while (!turn.static_actions.empty()) {
      while (ApplyActionUntilInterrupted(turn.static_actions.at(0), turn, battle)) {
        if (battle.HasEnded()) {
          return true;
        }

        if (!queued_commands.dynamic_commands.empty()) {
          return true;
        }

        if (!turn.dynamic_actions.empty()) {
          return true;
        }
      }
      turn.static_actions.erase(turn.static_actions.begin());
      const auto static_action_end_result = PostEvent<DefaultEvents::StaticActionEnd>({}, battle);
      if (static_action_end_result.has_value()) {
        const auto &static_action_end_actions = static_action_end_result.value();
        if (!static_action_end_actions.empty()) {
          turn.AddDynamicActions(static_action_end_actions);
          return true;
        }
      }
    }

    if (!queued_commands.static_commands.empty()) {
      const auto queued_static_command = queued_commands.static_commands.at(0);
      queued_commands.static_commands.erase(queued_commands.static_commands.begin());

      auto queued_static_action = TranslateAction(queued_static_command, battle);
      turn.AddStaticAction(queued_static_action);
      return true;
    }

    // TODO: replace this if constexpr stuff and implement a Schedule that contains Actionable objects
    // using Schedule = std::vector<std::vector<Action | Command | PlayerIndex | ActionablePlayerQuery()>>
    if constexpr (std::is_same_v<TCommandRequestStrategy, ImmediateCommands>) {
      if (!remaining_immediate_commands_.empty()) {

        const auto commands = battle.RequestCommands({remaining_immediate_commands_});
        if (!commands.empty()) {
          turn.AddStaticActions(commands);
          return true;
        }
      }
    }

    return false;
  }

  EventHandler<TBattle, TCommand, TEvents> event_handlers_;

  TActionTranslator action_translator_;
  TBattleEndChecker check_battle_ended_;

  // TODO: conditionally remove this variable depending on the command request strategy
  std::vector<std::size_t> remaining_immediate_commands_;
};
} // namespace ngl::tbc

#endif