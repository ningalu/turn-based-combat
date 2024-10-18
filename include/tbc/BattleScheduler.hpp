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
template <typename TUnit, typename TState, typename TCommand, typename TEvents>
class BattleScheduler {
  using TCommandPayload = typename TCommand::Payload;
  using TBattle         = Battle<TUnit, TState, TCommand>;
  using TAction         = Action<TBattle, TEvents, TCommand>;
  using TTurn           = Turn<TBattle, TEvents, TCommand>;

  using TActionTranslator = std::function<TAction(const TCommand &, const TBattle &)>;
  using TBattleEndChecker = std::function<std::optional<std::vector<std::size_t>>(const TBattle &)>;

public:
  template <typename TSpecificEvent>
  void SetHandler(std::function<std::vector<TAction>(TSpecificEvent, TBattle &)> handler) {
    event_handlers_.RegisterHandler<TSpecificEvent>(handler);
  }

  template <typename TSpecificEvent>
  [[nodiscard]] std::optional<std::vector<TAction>> PostEvent(const TSpecificEvent &e, TBattle &b) const {
    std::optional<std::vector<TAction>> out = std::nullopt;
    if (event_handlers_.HasHandler<TSpecificEvent>()) {
      out = event_handlers_.PostEvent<TSpecificEvent>(e, b);
    }
    return out;
  }

  [[nodiscard]] std::optional<std::vector<TAction>> PostEvent(const TEvents &e, TBattle &b) const {
    return std::visit([&, this](auto &&event) {
      using T = std::decay_t<decltype(event)>;
      return PostEvent<T>(event, b);
    },
                      e.payload);
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

  [[nodiscard]] std::vector<std::size_t> RunBattle(TBattle &b) {
    const auto player_indices = [&, this]() {
      std::vector<std::size_t> out(b.PlayerCount());
      std::iota(out.begin(), out.end(), std::size_t{0});
      return out;
    }();

    for (std::size_t i = 0; !b.HasEnded(); i++) {
      std::cout << "Start turn " << i + 1 << "\n";
      b.StartTurn();
      b.queued_commands.at(0).static_commands = b.OrderCommands(b.queued_commands.at(0).static_commands);
      Turn<TBattle, TEvents, TCommand> turn;
      if (i == 0) {
        auto event_action = PostEvent(DefaultEvents::BattleStart{}, b);
        if (event_action.has_value()) {
          turn.AddDynamicActions(event_action.value());
        }
      }
      RunTurn(turn, b);

      b.NextTurn();
    }

    return b.GetWinners();
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

      [[maybe_unused]] auto [status, winners, commands, events] = result.value();

      if (status == EffectResult::Status::STOP) {
        action.Cancel();
      }

      // if the effect caused the battle to end, report an interruption immediately
      // battle ending can sometimes still require effects to continue to be applied? TODO: research
      if (winners.winners.size() > 0) {
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
          const auto event_action_values = event_action.value();
          new_dynamic_actions.insert(new_dynamic_actions.begin(), event_action_values.begin(), event_action_values.end());
        }
      }

      if (new_dynamic_actions.size() > 0) {
        turn.AddDynamicActions(new_dynamic_actions);
        return true;
      }
    }
    return false;
  }

  [[nodiscard]] bool RunTurnUntilInterrupted(TTurn &turn, TBattle &battle) {
    assert(battle.queued_commands.size() > 0);
    auto &queued_commands = battle.queued_commands.at(0);

    while (turn.dynamic_actions.size() > 0) {
      while (ApplyActionUntilInterrupted(turn.dynamic_actions.at(0), turn, battle)) {
        if (battle.HasEnded()) {
          return true;
        }
      }
      turn.dynamic_actions.erase(turn.dynamic_actions.begin());
    }

    if (queued_commands.dynamic_commands.size() > 0) {
      const auto queued_dynamic_command = queued_commands.dynamic_commands.at(0);
      queued_commands.dynamic_commands.erase(queued_commands.dynamic_commands.begin());

      auto queued_dynamic_action = TranslateActions({queued_dynamic_command}, battle);
      turn.AddDynamicActions(queued_dynamic_action);
      return true;
    }

    while (turn.static_actions.size() > 0) {
      while (ApplyActionUntilInterrupted(turn.static_actions.at(0), turn, battle)) {
        if (battle.HasEnded()) {
          return true;
        }

        if (queued_commands.dynamic_commands.size() > 0) {
          return true;
        }

        if (turn.dynamic_actions.size() > 0) {
          return true;
        }
      }
      turn.static_actions.erase(turn.static_actions.begin());
      const auto static_action_end_result = PostEvent<DefaultEvents::StaticActionEnd>({}, battle);
      if (static_action_end_result.has_value()) {
        const auto &static_action_end_actions = static_action_end_result.value();
        if (static_action_end_actions.size() > 0) {
          turn.AddDynamicActions(static_action_end_actions);
          return true;
        }
      }
    }

    if (queued_commands.static_commands.size() > 0) {
      const auto queued_static_command = queued_commands.static_commands.at(0);
      queued_commands.static_commands.erase(queued_commands.static_commands.begin());

      auto queued_static_action = TranslateActions({queued_static_command}, battle);
      turn.AddStaticActions(queued_static_action);
      return true;
    }

    return false;
  }

  EventHandler<TBattle, TCommand, TEvents> event_handlers_;

  TActionTranslator action_translator_;
  TBattleEndChecker check_battle_ended_;
};
} // namespace ngl::tbc

#endif