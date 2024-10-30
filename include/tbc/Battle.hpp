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

#include "tbc/Actionable.hpp"
#include "tbc/Command.hpp"
#include "tbc/Comms.hpp"
#include "tbc/Log.h"
#include "tbc/PlayerCommandRequest.hpp"
#include "tbc/PlayerComms.hpp"
#include "tbc/Schedule.hpp"

namespace ngl::tbc {

template <
  typename TState,
  typename TCommand,
  typename TCommandResult,
  typename TEvent,
  SimultaneousActionStrategy TSimultaneousActionStrategy>
class Battle {
  using TBattle                = Battle<TState, TCommand, TCommandResult, TEvent, TSimultaneousActionStrategy>;
  using TComms                 = Comms<TBattle, TCommand, TCommandResult>;
  using TPlayerComms           = PlayerComms<TBattle, TCommand, TCommandResult>;
  using TCommandPayloadTypeSet = typename TCommand::PayloadTypeSet;
  using TPlayerCommandRequest  = PlayerCommandRequest<TCommand>;
  using TActionable            = Actionable<TCommand, TEvent, TSimultaneousActionStrategy>;
  using TSchedule              = Schedule<TCommand, TEvent, TSimultaneousActionStrategy>;
  using TCommandPayload        = typename TCommand::Payload;

  // TODO: this probably shouldnt return a valid set of commands anymore, just a status? or pairs of commands it saw and the related status
  using TCommandValidator        = std::function<std::pair<std::optional<std::vector<TCommandPayload>>, TCommandResult>(std::size_t, std::vector<TCommandPayload>, const TBattle &)>;
  using TCommandOrderer          = std::function<std::vector<TCommand>(const std::vector<TCommand> &, const TBattle &)>;
  using TTurnStartCommandChecker = std::function<TCommandPayloadTypeSet(std::size_t, const TBattle &)>;

public:
  constexpr static auto SimultaneousActionStrategy = TSimultaneousActionStrategy;

  Battle(const TState &state_, std::vector<TPlayerComms> comms)
      : state{state_}, comms_{std::move(comms)} {}

  explicit Battle(const std::vector<TPlayerComms> &comms)
      : Battle{{}, comms} {}

  TState state;
  // TODO: can you queue anything other than commands?
  // TODO: queue Actionables?
  std::vector<std::vector<TCommand>> queued_commands;
  TSchedule current_turn_schedule;

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
    const std::vector<std::size_t> &players,
    std::size_t attempts = 1000 // NOLINT related to #24
  ) {
    std::vector<TPlayerCommandRequest> requests;
    requests.reserve(players.size());
    for (const auto player : players) {
      requests.push_back({player, TCommandPayloadTypeSet{true}});
    }
    return RequestCommands(requests, nullptr, attempts);
  }

  [[nodiscard]] std::vector<TCommand> RequestCommands(
    const std::vector<TPlayerCommandRequest> &players,
    std::size_t attempts
  ) {
    return RequestCommands(players, nullptr, attempts);
  }

  [[nodiscard]] std::vector<TCommand> RequestCommands(
    const std::vector<TPlayerCommandRequest> &players,
    TCommandValidator validator = nullptr,
    std::size_t attempts        = 1000 // NOLINT related to #24
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
          const auto incoming_payloads = comms_.players.at(player).RequestCommands(valid_commands, *this).get();
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
    queued_commands.at(turns_ahead).push_back(c);
  }

  void PostLog(const std::string &message) {
    comms_.PostLog(message);
  }

  void InitTurn(const TSchedule &schedule) {
    current_turn_schedule = schedule;
  }

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
  TComms comms_;

  // TODO: refactor
  std::optional<std::vector<std::size_t>> winner_indices_;

  TCommandValidator command_validator_;
  TCommandOrderer command_orderer_;
  TTurnStartCommandChecker valid_turn_start_commands_;
};
} // namespace ngl::tbc

#endif