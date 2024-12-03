#ifndef TBC_SCHEDULER_HPP
#define TBC_SCHEDULER_HPP

#include <cassert>
#include <functional>
#include <future>
#include <iostream>
#include <numeric>
#include <optional>
#include <stack>
#include <vector>

#include "util/oneof.hpp"
#include "util/tmp.hpp"

#include "tbc/Action.hpp"
#include "tbc/Battle.hpp"
#include "tbc/Command.hpp"
#include "tbc/Effect.hpp"
#include "tbc/EventHandler.hpp"
#include "tbc/PlayerCommandRequest.hpp"
#include "tbc/Schedule.hpp"

namespace ngl::tbc {

namespace detail {
template <typename TCommand, SimultaneousActionStrategy TSimultaneousActionStrategy>
struct ActionTranslatorHelper {};

template <typename TCommand>
struct ActionTranslatorHelper<TCommand, SimultaneousActionStrategy::DISABLED> {
  using ActionFrom = TCommand;
};

template <typename TCommand>
struct ActionTranslatorHelper<TCommand, SimultaneousActionStrategy::ENABLED> {
  using ActionFrom = std::vector<TCommand>;
};
} // namespace detail

template <typename TState, typename TCommand, typename TCommandError, typename TEvent, SimultaneousActionStrategy TSimultaneousActionStrategy>
class Scheduler {
  using TCommandPayload = typename TCommand::Payload;
  using TBattle         = Battle<TState, TCommand, TCommandError, TEvent, TSimultaneousActionStrategy>;
  using TSchedule       = Schedule<TCommand, TEvent, TSimultaneousActionStrategy>;
  using TEffect         = Effect<TBattle, TCommand, TEvent>;
  using TAction         = Action<TBattle, TCommand, TEvent>;
  using TActionable     = Actionable<TCommand, TEvent, TSimultaneousActionStrategy>;

  using TCommandsToActions = typename detail::ActionTranslatorHelper<TCommand, TSimultaneousActionStrategy>::ActionFrom;

  [[nodiscard]] std::vector<TAction> GenerateSequentialActions(const TActionable &actionable, TBattle &battle) {
    return match(
      actionable,
      [&battle, this](const TCommand &command) {
        return std::vector{TranslateAction(command, battle)};
      },
      [&battle, this](std::size_t player) {
        const auto commands = battle.RequestCommands(std::vector{player});
        return TranslateActions(commands, battle);
      },
      [&battle, this](const TEvent &event) {
        const auto actions_opt = PostEvent(event, battle);
        if (actions_opt.has_value()) {
          return actions_opt.value();
        }
        return std::vector<TAction>{};
      }
    );
  }

public:
  // TODO: genericise the singular/multiple actionables/commands idea
  using TActionTranslator = std::function<TAction(const TCommandsToActions &, const TBattle &)>;
  using TBattleEndChecker = std::function<std::optional<std::vector<std::size_t>>(const TBattle &)>;

  using TScheduleGenerator = std::function<TSchedule(TBattle &, const std::vector<TCommand> &, std::size_t)>;

  template <typename TSpecificEvent>
  void SetHandler(std::function<std::vector<TAction>(TSpecificEvent, TBattle &)> handler) {
    event_handlers_.template RegisterHandler<TSpecificEvent>(handler);
  }

  template <typename TSpecificEvent>
  [[nodiscard]] std::optional<std::vector<TAction>> PostEvent(const TSpecificEvent &e, TBattle &b) const {
    std::optional<std::vector<TAction>> out = std::nullopt;
    if (event_handlers_.template HasHandler<TSpecificEvent>()) {
      out = event_handlers_.template PostEvent<TSpecificEvent>(e, b);
    }
    return out;
  }

  [[nodiscard]] std::optional<std::vector<TAction>> PostEvent(const TEvent &event, TBattle &battle) const {
    return std::visit(
      [&](auto &&event_payload) {
        using T = std::decay_t<decltype(event_payload)>;
        return PostEvent<T>(event_payload, battle);
      },
      event.payload
    );
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

  [[nodiscard]] TAction TranslateAction(const TCommandsToActions &command, const TBattle &battle) const {
    assert(action_translator_);
    return action_translator_(command, battle);
  }

  [[nodiscard]] std::vector<TAction> TranslateActions(const std::vector<TCommandsToActions> &commands, const TBattle &battle) const {
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
    ResolveEvent<DefaultEvents::ScheduleStart>({}, battle);

    while ((!battle.current_turn_schedule.Empty()) && (!battle.HasEnded())) {
      const auto actionable = battle.current_turn_schedule.order.at(0);
      ResolveEvent<DefaultEvents::PlannedActionStart>({}, battle);
      // TODO: figure out configurable simultaneous/sequential actionable resolution
      // TODO: everything but Commands can return multiple actions. figure out if anything special
      // should happen with these or if they should just be resolved in order
      const auto actions = [&, this]() {
        if constexpr (TSimultaneousActionStrategy == SimultaneousActionStrategy::DISABLED) {
          return GenerateSequentialActions(actionable, battle);
        } else {
          // Actionable is std::vector<command | player | event>
          // TODO: how to deal with this? new callbac kfor this scenario?
          std::vector<TCommand> actionable_commands;
          for (const auto actionable_item : actionable) {
            match(
              actionable_item,
              [&actionable_commands](const TCommand &command) {
                actionable_commands.push_back(command);
              },
              [&actionable_commands, &battle, this](std::size_t player) {
                const auto player_commands = battle.RequestCommands(std::vector{player});
                actionable_commands.insert(actionable_commands.end(), player_commands.begin(), player_commands.end());
              },
              []([[maybe_unused]] const TEvent &event) {
                throw std::logic_error{"TODO: who knows what this should mean"};
              }
            );
          }
          return std::vector{TranslateAction(actionable_commands, battle)};
        }
      }();

      for (const auto &action : actions) {
        ResolveAction(action, battle);
      }
      battle.current_turn_schedule.Next();

      ResolveEvent<DefaultEvents::PlannedActionEnd>({}, battle);
    }

    // TODO: temp, figure out how to control post battle effects
    if (!battle.HasEnded()) {
      ResolveEvent<DefaultEvents::ScheduleEnd>({}, battle);
    }
  }

  void ResolveAction(const TAction &action, TBattle &battle, std::size_t max_depth = 100) { // NOLINT relates to #24
    std::stack<TAction> to_resolve;
    to_resolve.push(action);

    std::size_t depth = 0;
    while ((!to_resolve.empty()) && (depth < max_depth)) {
      depth++;

      const auto res = to_resolve.top().ApplyNext(battle);

      // if the current action is resolved, try the next
      if (!res.has_value()) {
        to_resolve.pop();
        continue;
      }

      const auto &[status, winners, actionables] = res.value();

      // Resolve the current action even if the battle has ended
      // TODO: make this configurable
      // TODO: update this to match the current winner model
      if (winners.has_value()) {
        battle.EndBattle(winners.value());
      }

      // TODO: this is a placeholder. figure out something more flexible later
      if (status == EffectStatus::FAILED) {
        to_resolve.pop();
        continue;
      }

      if (!actionables.empty()) {
        for (auto it = actionables.rbegin(); it != actionables.rend(); it++) {
          if constexpr (TSimultaneousActionStrategy == SimultaneousActionStrategy::DISABLED) {

            const auto dynamic_actions = GenerateSequentialActions(*it, battle);
            for (auto jt = dynamic_actions.rbegin(); jt != dynamic_actions.rend(); jt++) {
              to_resolve.push(*jt);
            }
          } else {
            // TODO: as above
            std::vector<TCommand> actionable_commands;
            // TODO: naming is getting hard at this scope
            for (const auto actable : *it) {
              actionable_commands.push_back(std::get<TCommand>(actable));
            }
            to_resolve.push(TranslateAction(actionable_commands, battle));
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