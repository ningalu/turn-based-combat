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
#include "tbc/CommandResponse.hpp"
#include "tbc/Comms.hpp"
#include "tbc/Log.h"
#include "tbc/PlayerCommandRequest.hpp"
#include "tbc/PlayerComms.hpp"
#include "tbc/Schedule.hpp"

namespace ngl::tbc {

template <
  typename TState,
  typename TCommand,
  typename TCommandError,
  typename TEvent,
  SimultaneousActionStrategy TSimultaneousActionStrategy>
class Battle {
public:
  using TBattle                = Battle<TState, TCommand, TCommandError, TEvent, TSimultaneousActionStrategy>;
  using TComms                 = Comms<TBattle, TCommand, TCommandError>;
  using TPlayerComms           = PlayerComms<TBattle, TCommand, TCommandError>;
  using TCommandPayloadTypeSet = typename TCommand::PayloadTypeSet;
  using TPlayerCommandRequest  = PlayerCommandRequest<TCommand>;
  using TCommandResponse       = CommandResponse<TCommandError>;
  using TActionable            = Actionable<TCommand, TEvent, TSimultaneousActionStrategy>;
  using TSchedule              = Schedule<TCommand, TEvent, TSimultaneousActionStrategy>;
  using TCommandPayload        = typename TCommand::Payload;

  using TCommandPreprocessor     = std::function<std::vector<TCommandPayload>(std::size_t, const std::vector<TCommandPayload> &, TBattle)>;
  using TCommandValidator        = std::function<TCommandResponse(std::size_t, const std::vector<TCommandPayload> &, const TBattle &)>;
  using TCommandOrderer          = std::function<std::vector<TCommand>(const std::vector<TCommand> &, const TBattle &)>;
  using TTurnStartCommandChecker = std::function<TCommandPayloadTypeSet(std::size_t, const TBattle &)>;

  using State         = TState;
  using Command       = TCommand;
  using CommandResult = TCommandError;
  using Event         = TEvent;

  constexpr static auto SimultaneousActionStrategy = TSimultaneousActionStrategy;

  TState state;
  // TODO: can you queue anything other than commands?
  // TODO: queue Actionables?
  std::vector<std::vector<TCommand>> queued_commands;
  TSchedule current_turn_schedule;

  TCommandPreprocessor command_preprocessor;
  TCommandValidator command_validator;
  TCommandOrderer command_orderer;

  Battle(const TState &state_, std::vector<TPlayerComms> comms)
      : state{state_}, comms_{std::move(comms)} {}

  explicit Battle(const std::vector<TPlayerComms> &comms)
      : Battle{{}, comms} {}

  [[nodiscard]] std::size_t PlayerCount() const {
    return comms_.PlayerCount();
  }

  void SetCommandValidator(const TCommandValidator &validator) {
    command_validator = validator;
  }

  [[nodiscard]] std::vector<TCommandPayload> PreprocessCommands(std::size_t authority, const std::vector<TCommandPayload> &commands) const {
    return command_preprocessor ? command_preprocessor(authority, commands, *this) : commands;
  }

  // TODO: is there actually any meaningful domain-agnostic validation that can be done here?
  [[nodiscard]] TCommandResponse ValidateCommands(std::size_t authority, const std::vector<TCommandPayload> &commands) const {
    assert(command_validator);
    return command_validator(authority, commands, *this);
  }

  [[nodiscard]] std::vector<TCommand> OrderCommands(const std::vector<TCommand> &commands) const {
    return command_orderer ? command_orderer(commands, *this) : commands;
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
    TCommandValidator validator = nullptr,
    std::size_t attempts        = 1000 // NOLINT related to #24
  ) {
    comms_.Flush();

    std::vector<std::future<std::vector<TCommand>>> action_handles;
    for (const auto &request : players) {
      const auto &player = request.player;
      assert(comms_.PlayerCount() > player);
      const auto &valid_commands = request.valid_commands;

      // Async poll each player for static commands, rejecting and repolling if any are invalid
      action_handles.push_back(std::async(std::launch::async, [=, this]() {
        std::optional<std::vector<TCommandPayload>> payloads;

        for (std::size_t i = 0; i < attempts; i++) {
          const auto incoming_payloads     = comms_.players.at(player).RequestCommands(valid_commands, *this).get();
          const auto preprocessed_commands = PreprocessCommands(player, incoming_payloads);
          const auto validation_result     = validator ? validator(player, preprocessed_commands, *this) : ValidateCommands(player, preprocessed_commands);

          // Clang can't capture structured bindings in closures; assign
          comms_.players.at(player).RespondToCommands(validation_result);
          if (validation_result.pass) {
            payloads = preprocessed_commands;
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
    while (queued_commands.size() < (turns_ahead + 1)) {
      queued_commands.push_back({});
    }
    queued_commands.at(turns_ahead).push_back(c);
  }

  void PostLog(const std::string &message) {
    comms_.PostLog(message);
  }

  void InitTurn(const TSchedule &schedule) {
    current_turn_schedule = schedule;
    if (!queued_commands.empty()) {
      queued_commands.erase(queued_commands.begin());
    }
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

  std::optional<std::vector<std::size_t>> winner_indices_ = std::nullopt;
};
} // namespace ngl::tbc

#endif