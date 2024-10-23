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
#include "tbc/CommandQueue.hpp"
#include "tbc/Layout.h"
#include "tbc/PlayerCommandRequest.hpp"
#include "tbc/PlayerComms.hpp"

namespace ngl::tbc {
struct BattleLog {
  explicit BattleLog(std::size_t num_players);
  BattleLog(std::size_t num_players, const std::string &default_message);

  void Insert(std::size_t player, const std::string &message);
  void Insert(const std::unordered_set<std::size_t> &players, const std::string &message);
  void Insert(std::nullopt_t spectator, const std::string &message);
  void Insert(std::optional<std::size_t> spectator, const std::string &message);
  void Insert(const std::unordered_set<std::optional<std::size_t>> &players, const std::string &message);

  [[nodiscard]] std::optional<const std::string *> Retrieve(std::size_t player) const;
  [[nodiscard]] std::optional<const std::string *> Retrieve(std::nullopt_t spectator) const;

  void Clear(std::size_t player);
  void Clear(std::nullopt_t spectator);

  std::unordered_map<std::optional<std::size_t>, const std::string *> distribution;
  std::unordered_set<std::string> messages;
};

template <typename TState, typename TCommand, typename TCommandResult>
class Battle {
  using TBattle                = Battle<TState, TCommand, TCommandResult>;
  using TPlayerComms           = PlayerComms<TCommand, TCommandResult>;
  using TCommandPayloadTypeSet = typename TCommand::PayloadTypeSet;

  using TCommandPayload          = typename TCommand::Payload;
  using TCommandValidator        = std::function<std::pair<std::optional<std::vector<TCommandPayload>>, TCommandResult>(std::size_t, std::vector<TCommandPayload>, const TBattle &)>;
  using TCommandOrderer          = std::function<std::vector<TCommand>(const std::vector<TCommand> &, const TBattle &)>;
  using TTurnStartCommandChecker = std::function<TCommandPayloadTypeSet(std::size_t, const TBattle &)>;

  using TLogHandler = typename TPlayerComms::TLogHandler;

public:
  using Log = BattleLog;

  Battle(const TState &state_, std::size_t seed, std::vector<TPlayerComms> comms, Layout layout)
      : state{state_}, seed_{seed}, comms_{std::move(comms)}, layout_{std::move(layout)} {}

  Battle(std::size_t seed, const std::vector<TPlayerComms> &comms, const Layout &layout)
      : Battle{{}, seed, comms, layout} {}

  TState state;
  std::vector<std::vector<TCommand>> queued_commands;
  CommandQueue<TCommand> current_turn_commands;

  [[nodiscard]] const Layout &layout() const {
    return layout_;
  }

  [[nodiscard]] Slot GetSlot(Slot::Index i) const {
    return layout_.GetSlot(i);
  }

  [[nodiscard]] std::size_t PlayerCount() const {
    return comms_.size();
  }

  void SetSpectatorLogHandler(TLogHandler handler) { spectator_log_output_ = handler; }

  void SetPlayerLogHandler(std::size_t player, TLogHandler handler) {
    if (player >= comms_.size()) {
      throw std::out_of_range{"Assigning Log handler to invalid player index: " + std::to_string(player)};
    }
    comms_.at(player).SetLogHandler(handler);
  }

  void SetMasterLogHandler(TLogHandler handler) { master_log_output_ = handler; }

  void PostLog(const std::string &message) {
    log_buffer_.push_back(Log{comms_.size(), message});
  }

  void PostLog(std::optional<std::size_t> player, const std::string &message) {
    Log log{comms_.size()};
    log.Insert(player, message);
    log_buffer_.push_back(log);
  }

  void PostLog(const std::unordered_set<std::optional<std::size_t>> &players, const std::string &message) {
    Log log{comms_.size()};
    log.Insert(players, message);
    log_buffer_.push_back(log);
  }

  void PostLog(const Log &log) {
    log_buffer_.push_back(log);
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

  [[nodiscard]] CommandQueue<TCommand> RequestCommands(
    const std::vector<PlayerCommandRequest<TCommand>> &players,
    TCommandValidator validator = nullptr,
    std::size_t attempts        = 1
  ) {
    // TODO: post logs asynchronously
    for (const auto &log : log_buffer_) {
      for (std::size_t i = 0; i < comms_.size(); i++) {
        const auto message = log.Retrieve(i);
        if (message.has_value()) {
          comms_.at(i).PostLog(*message.value());
        }
      }
      const auto spectator_message = log.Retrieve(std::nullopt);
      if (spectator_message.has_value()) {
        spectator_log_output_(*spectator_message.value());
      }
      log_buffer_.erase(log_buffer_.begin());
    }

    std::vector<std::future<std::vector<TCommand>>> action_handles;

    // Clang cant capture structured bindings in closures????????? Are they stupid?????????????????????????????????????
    for (const auto &request : players) {
      const auto &player         = request.player;
      const auto &valid_commands = request.valid_commands;

      assert(comms_.size() > player);

      // Async poll each player for static commands, rejecting and repolling if any are invalid
      action_handles.push_back(std::async(std::launch::async, [=, this]() {
        std::optional<std::vector<TCommandPayload>> payloads;
        TCommandResult result;

        for (std::size_t i = 0; i < attempts; i++) {
          const auto incoming_payloads = comms_.at(player).RequestCommands(valid_commands).get();
          if (validator) {
            auto validation_result = validator(player, incoming_payloads, *this);
            payloads               = validation_result.first;
            result                 = validation_result.second;
          } else {
            auto validation_result = ValidateCommands(player, incoming_payloads);
            payloads               = validation_result.first;
            result                 = validation_result.second;
          }
          comms_.at(player).RequestCommandsResponse(result);
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
    return {{}, actions};
  }

  void BufferCommand(const TCommand &c, std::size_t turns_ahead) {
    assert(turns_ahead != 0);
    while (queued_commands.size() < (turns_ahead + 1)) {
      queued_commands.push_back({});
    }
    std::cout << "Queued " << turns_ahead << " turns ahead\n";
    queued_commands.at(turns_ahead).BufferCommand(c);
  }

  void StartTurn(const std::vector<PlayerCommandRequest<TCommand>> &scheduled_command_requests) {
    current_turn_commands.ClearCommands();

    if (!queued_commands.empty()) {
      current_turn_commands.static_commands = std::move(queued_commands.at(0));
      queued_commands.erase(queued_commands.begin());
    }

    // const auto player_indices = [&, this]() {
    //   std::vector<std::pair<std::size_t, TCommandPayloadTypeSet>> out(PlayerCount());
    //   for (std::size_t i = 0; i < out.size(); i++) {
    //     out.at(i).first  = i;
    //     out.at(i).second = GetValidTurnStartCommands(i);
    //   }
    //   return out;
    // }();

    const auto scheduled_commands = RequestCommands(scheduled_command_requests);
    current_turn_commands.Merge(scheduled_commands);
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
  std::size_t seed_;

  // std::queue? any reason to look at past logs?
  std::vector<Log> log_buffer_;
  TLogHandler master_log_output_;
  std::vector<TPlayerComms> comms_;
  TLogHandler spectator_log_output_;

  Layout layout_;
  // TODO: refactor
  std::optional<std::vector<std::size_t>> winner_indices_;

  TCommandValidator command_validator_;
  TCommandOrderer command_orderer_;
  TTurnStartCommandChecker valid_turn_start_commands_;
};
} // namespace ngl::tbc

#endif