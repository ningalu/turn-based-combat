#ifndef TBC_BATTLE_H
#define TBC_BATTLE_H

#include <cassert>
#include <cstdint>
#include <future>
#include <iostream>
#include <numeric>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "tbc/Command.hpp"
#include "tbc/Comms.hpp"
#include "tbc/Layout.h"
#include "tbc/Log.h"
#include "tbc/PlayerCommandRequest.hpp"
#include "tbc/PlayerComms.hpp"

namespace ngl::tbc {

template <typename TState, typename TCommand, typename TCommandResult>
class Battle {
  using TBattle                = Battle<TState, TCommand, TCommandResult>;
  using TPlayerComms           = PlayerComms<TCommand, TCommandResult>;
  using TCommandPayloadTypeSet = typename TCommand::PayloadTypeSet;

  using TCommandPayload          = typename TCommand::Payload;
  using TCommandValidator        = std::function<std::pair<std::optional<std::vector<TCommandPayload>>, TCommandResult>(std::size_t, std::vector<TCommandPayload>, const TBattle &)>;
  using TCommandOrderer          = std::function<std::vector<TCommand>(const std::vector<TCommand> &, const TBattle &)>;
  using TTurnStartCommandChecker = std::function<TCommandPayloadTypeSet(std::size_t, const TBattle &)>;

public:
  // is an event actionable? probably right?
  using Actionable = std::variant<TCommand, std::size_t, std::function<std::size_t(const TBattle &)>>;

  struct Schedule {
    std::vector<std::vector<Actionable>> order;
    Schedule() = default;
    Schedule(std::vector<TCommand> commands) {
      for (const auto &command : commands) {
        order.push_back(std::vector<Actionable>{Actionable{command}});
      }
    }
    [[nodiscard]] bool Empty() const {
      return order.empty();
    }
    void Next() {
      assert(!Empty());
      order.erase(order.begin());
    }
  };

  Battle(const TState &state_, std::vector<TPlayerComms> comms, Layout layout)
      : state{state_}, comms_{std::move(comms)}, layout_{std::move(layout)} {}

  Battle(const std::vector<TPlayerComms> &comms, const Layout &layout)
      : Battle{{}, comms, layout} {}

  TState state;
  // TODO: can you queue anything other than commands?
  std::vector<std::vector<TCommand>> queued_commands;
  Schedule current_turn_schedule;

  [[nodiscard]] const Layout &layout() const {
    return layout_;
  }

  [[nodiscard]] Slot GetSlot(Slot::Index i) const {
    return layout_.GetSlot(i);
  }

  [[nodiscard]] std::size_t PlayerCount() const {
    return comms_.PlayerCount();
  }

  void SetCommandValidator(const TCommandValidator &validator) {
    command_validator_ = validator;
  }

  // TODO: is there actually any meaningful domain-agnostic validation that can be done here?
  [[nodiscard]] std::pair<std::optional<std::vector<TCommandPayload>>, TCommandResult> ValidateCommands(std::size_t authority, const std::vector<TCommandPayload> &commands) const {
    assert(command_validator_);
    return command_validator_(authority, commands, *this);
  }

  void SetCommandOrderer(const TCommandOrderer &orderer) {
    command_orderer_ = orderer;
  }

  [[nodiscard]] std::vector<TCommand> OrderCommands(const std::vector<TCommand> &commands) const {
    return command_orderer_ ? command_orderer_(commands, *this) : commands;
  }

  void SetTurnStartCommands(const TTurnStartCommandChecker &checker) {
    valid_turn_start_commands_ = checker;
  }

  [[nodiscard]] TCommandPayloadTypeSet GetValidTurnStartCommands(std::size_t player) const {
    return (valid_turn_start_commands_ ? valid_turn_start_commands_(player, *this) : TCommandPayloadTypeSet{true});
  }

  [[nodiscard]] std::vector<TCommand> RequestCommands(
    const std::vector<PlayerCommandRequest<TCommand>> &players,
    std::size_t attempts
  ) {
    return RequestCommands(players, nullptr, attempts);
  }

  [[nodiscard]] std::vector<TCommand> RequestCommands(
    const std::vector<PlayerCommandRequest<TCommand>> &players,
    TCommandValidator validator = nullptr,
    std::size_t attempts        = 1
  ) {
    comms_.Flush();

    std::vector<std::future<std::vector<TCommand>>> action_handles;
    // Clang cant capture structured bindings in closures????????? Are they stupid?????????????????????????????????????
    for (const auto &request : players) {
      const auto &player = request.player;
      assert(comms_.PlayerCount() > player);
      const auto &valid_commands = request.valid_commands;

      // Async poll each player for static commands, rejecting and repolling if any are invalid
      action_handles.push_back(std::async(std::launch::async, [=, this]() {
        std::optional<std::vector<TCommandPayload>> payloads;
        TCommandResult result;

        for (std::size_t i = 0; i < attempts; i++) {
          const auto incoming_payloads = comms_.players.at(player).RequestCommands(valid_commands).get();
          auto validation_result       = validator ? validator(player, incoming_payloads, *this) : ValidateCommands(player, incoming_payloads);
          payloads                     = validation_result.first;
          result                       = validation_result.second;
          comms_.players.at(player).RespondToCommands(result);
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

  void BufferCommand(const TCommand &c, std::size_t turns_ahead) {
    assert(turns_ahead != 0);
    while (queued_commands.size() < (turns_ahead + 1)) {
      queued_commands.push_back({});
    }
    std::cout << "Queued " << turns_ahead << " turns ahead\n";
    queued_commands.at(turns_ahead).BufferCommand(c);
  }

  void InitTurn(const Schedule &schedule) {
    current_turn_schedule = schedule;
  }

  // void StartTurn(const std::vector<PlayerCommandRequest<TCommand>> &scheduled_command_requests) {
  //   current_turn_commands.ClearCommands();
  //   current_turn_schedule.order.clear();

  //   if (!queued_commands.empty()) {
  //     current_turn_commands.static_commands = std::move(queued_commands.at(0));
  //     queued_commands.erase(queued_commands.begin());
  //   }

  //   const auto scheduled_commands = RequestCommands(scheduled_command_requests);
  //   current_turn_commands.Merge({{}, scheduled_commands});
  // }

  // void StartTurn() {
  //   current_turn_commands.ClearCommands();

  //   if (!queued_commands.empty()) {
  //     current_turn_commands.static_commands = std::move(queued_commands.at(0));
  //     queued_commands.erase(queued_commands.begin());
  //   }
  // }

  void EndBattle(const std::vector<std::size_t> &winners) {
    winner_indices_ = winners;
  }

  [[nodiscard]] bool HasEnded() const {
    return winner_indices_ != std::nullopt;
  }

  [[nodiscard]] std::vector<std::size_t> GetWinners() const {
    return winner_indices_.value();
  }

protected:
  Comms<TCommand, TCommandResult> comms_;

  // TODO: move to domain helpers
  Layout layout_;

  // TODO: refactor
  std::optional<std::vector<std::size_t>> winner_indices_;

  TCommandValidator command_validator_;
  TCommandOrderer command_orderer_;
  TTurnStartCommandChecker valid_turn_start_commands_;
};
} // namespace ngl::tbc

#endif