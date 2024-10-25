#ifndef TBC_BATTLESCHEDULER_HPP
#define TBC_BATTLESCHEDULER_HPP

#include <cassert>
#include <functional>
#include <future>
#include <iostream>
#include <numeric>
#include <optional>
#include <stack>
#include <vector>

#include "util/tmp.hpp"

#include "tbc/Action.hpp"
#include "tbc/Battle.hpp"
#include "tbc/Command.hpp"
#include "tbc/Effect.hpp"
#include "tbc/EventHandler.hpp"
#include "tbc/PlayerCommandRequest.hpp"

namespace ngl::tbc {

template <typename TState, typename TCommand, typename TCommandResult, typename TEvent>
class BattleScheduler {
  using TCommandPayload = typename TCommand::Payload;
  using TBattle         = Battle<TState, TCommand, TCommandResult, TEvent>;
  using TSchedule       = typename TBattle::Schedule;
  using TAction         = Action<TBattle, TEvent, TCommand>;

  using TActionTranslator = std::function<TAction(const TCommand &, const TBattle &)>;
  using TBattleEndChecker = std::function<std::optional<std::vector<std::size_t>>(const TBattle &)>;

  using TScheduleGenerator = std::function<TSchedule(TBattle &, const std::vector<TCommand> &, std::size_t)>;

public:
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

  [[nodiscard]] std::optional<std::vector<TAction>> PostEvent(const TEvent &event, TBattle &battle) const {
    return std::visit([&](auto &&event_payload) {
      using T = std::decay_t<decltype(event_payload)>;
      return PostEvent<T>(event_payload, battle);
    },
                      event.payload);
  }

  // should only really be used for statically known event timings, since dynamically posted events need to be scheduled
  template <typename TSpecificEvent>
  void ResolveEvent(const TSpecificEvent &event, TBattle &battle) {
    const auto actions = PostEvent(event, battle);
    if (actions.has_value()) {
      for (const auto &action : actions.value()) {
        ResolveAction(action, battle);
        if (battle.HasEnded()) {
          return;
        }
      }
    }
  }

  void SetScheduleGenerator(TScheduleGenerator generator) { schedule_generator_ = std::move(generator); }
  [[nodiscard]] TSchedule GenerateSchedule(TBattle &battle, const std::vector<TCommand> &buffered_commands, std::size_t turn) {
    assert(schedule_generator_);
    return schedule_generator_(battle, buffered_commands, turn);
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

  [[nodiscard]] std::vector<std::size_t> RunBattle(TBattle &battle) {
    for (std::size_t i = 0; !battle.HasEnded(); i++) {
      std::cout << "Start turn " << i + 1 << "\n";

      battle.InitTurn(GenerateSchedule(battle, battle.queued_commands.empty() ? std::vector<TCommand>{} : battle.queued_commands.at(0), i));

      if (i == 0) {
        auto event_action = PostEvent(DefaultEvents::BattleStart{}, battle);
        if (event_action.has_value()) {
          for (const auto &action : event_action.value()) {
            ResolveAction(action, battle);
          }
        }
      }
      RunTurn(battle);
    }

    return battle.GetWinners();
  }

  void RunTurn(TBattle &battle) {
    ResolveEvent<DefaultEvents::TurnStart>({}, battle);

    while (!battle.current_turn_schedule.Empty()) {
      const auto actionable = battle.current_turn_schedule.order.at(0);
      // TODO: figure out configurable simultaneous/sequential actionable resolution
      assert(actionable.size() == 1);
      const auto action = std::visit([&](auto &&payload) {
        using T = std::decay_t<decltype(payload)>;
        if constexpr (std::is_same_v<T, TCommand>) {
          return TranslateAction(payload, battle);
        } else {
          // TODO: query immediate action actionables
          assert(false);
          return TAction{std::vector<typename TAction::Deferred>{}};
        }
      },
                                     actionable.at(0));
      ResolveAction(action, battle);
      battle.current_turn_schedule.Next();
    }

    // TODO: temp, figure out how to control post battle effects
    if (!battle.HasEnded()) {
      ResolveEvent<DefaultEvents::TurnEnd>({}, battle);
    }
  }

  void ResolveAction(const TAction &action, TBattle &battle, std::size_t max_depth = 100) {
    std::stack<TAction> to_resolve;
    to_resolve.push(action);

    std::size_t depth = 0;
    while ((to_resolve.size() > 0) && (depth < max_depth)) {
      depth++;

      const auto res = to_resolve.top().ApplyNext(battle);

      // if the current action is resolved, try the next
      if (!res.has_value()) {
        to_resolve.pop();
        continue;
      }

      const auto [status, winners, commands, events] = res.value();

      // Resolve the current action even if the battle has ended
      // TODO: make this configurable
      // TODO: update this to match the current winner model
      if (!winners.winners.empty()) {
        battle.EndBattle(winners.winners);
      }

      // TODO: this is a placeholder. figure out something more flexible later
      if (status == EffectResult::Status::FAILED) {
        to_resolve.pop();
        continue;
      }

      // TODO: this should be a vector of Actionable values
      if (!commands.empty()) {
        for (auto it = commands.rbegin(); it != commands.rend(); it++) {
          const auto dynamic_action = TranslateAction(*it, battle);
          to_resolve.push(dynamic_action);
        }
      }

      // TODO: see effect result TODO
      if (!events.events.empty()) {
        for (auto event = events.events.rbegin(); event != events.events.rend(); event++) {
          const auto event_actions_opt = PostEvent(*event, battle);
          // TODO: why is this optional?
          if (event_actions_opt.has_value()) {
            const auto event_actions = event_actions_opt.value();
            for (auto event_action = event_actions.rbegin(); event_action != event_actions.rend(); event_action++) {
              to_resolve.push(*event_action);
            }
          }
        }
      }
    }

    if (!(depth < max_depth)) {
      std::cout << "Exceeded maximum Action resolution depth\n";
    }
  }

protected:
  EventHandler<TBattle, TCommand, TEvent> event_handlers_;

  TScheduleGenerator schedule_generator_;

  TActionTranslator action_translator_;
  TBattleEndChecker check_battle_ended_;

  // TODO: conditionally remove this variable depending on the command request strategy
  std::vector<std::size_t> remaining_immediate_commands_;
};
} // namespace ngl::tbc

#endif