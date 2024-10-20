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
#include "tbc/PlayerComms.hpp"

namespace ngl::tbc {
struct BattleLog {
  BattleLog(std::size_t num_players);
  BattleLog(std::size_t num_players, const std::string &default_message);
  void Insert(std::size_t player, const std::string &message);
  void Insert(const std::unordered_set<std::size_t> &players, const std::string &message);
  void Insert(std::nullopt_t spectator, const std::string &message);
  void Insert(const std::unordered_set<std::optional<std::size_t>> &players, const std::string &message);
  [[nodiscard]] std::optional<const std::string *> Retrieve(std::size_t player);
  [[nodiscard]] std::optional<const std::string *> Retrieve(std::nullopt_t spectator);
  void Clear(std::size_t player);
  void Clear(std::nullopt_t spectator);
  std::unordered_map<std::optional<std::size_t>, const std::string *> distribution;
  std::unordered_set<std::string> messages;
};

template <typename TUnit, typename TState, typename TCommand, typename TCommandResult>
class Battle : public TState {
  using TBattle                = Battle<TUnit, TState, TCommand, TCommandResult>;
  using TPlayerComms           = PlayerComms<TCommand, TCommandResult>;
  using TCommandPayloadTypeSet = typename TCommand::PayloadTypeSet;

  using TCommandPayload          = typename TCommand::Payload;
  using TCommandValidator        = std::function<std::pair<std::optional<std::vector<TCommandPayload>>, TCommandResult>(std::size_t, std::vector<TCommandPayload>, const TBattle &)>;
  using TCommandOrderer          = std::function<std::vector<TCommand>(const std::vector<TCommand> &, const TBattle &)>;
  using TTurnStartCommandChecker = std::function<TCommandPayloadTypeSet(std::size_t, const TBattle &)>;

  using TLogHandler = typename TPlayerComms::TLogHandler;

public:
  using Log = BattleLog;

  Battle(const TState &state_, std::size_t seed, const std::vector<TPlayerComms> &comms, const Layout &layout) : TState{state_},
                                                                                                                 seed_{seed}, comms_{comms}, layout_{layout} {}
  Battle(std::size_t seed, const std::vector<TPlayerComms> &comms, const Layout &layout) : Battle{{}, seed, comms, layout} {}

  std::vector<CommandQueue<TCommand>> queued_commands;

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
    Log l{comms_.size()};
    l.Insert(player, message);
    log_buffer_.push_back(log);
  }
  void PostLog(const std::unordered_set<std::optional<std::size_t>> &players, const std::string &message) {
    Log l{comms_.size()};
    l.Insert(players, message);
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
    const std::vector<std::pair<std::size_t, TCommandPayloadTypeSet>> &players,
    std::size_t attempts
  ) {
    return RequestCommands(players, nullptr, attempts);
  }

  [[nodiscard]] CommandQueue<TCommand> RequestCommands(
    const std::vector<std::pair<std::size_t, TCommandPayloadTypeSet>> &players,
    TCommandValidator validator = nullptr,
    std::size_t attempts        = 1
  ) {
    std::vector<std::future<std::vector<TCommand>>> action_handles;

    for (const auto [player, valid_commands] : players) {
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

  void StartTurn() {
    const auto player_indices = [&, this]() {
      std::vector<std::pair<std::size_t, TCommandPayloadTypeSet>> out(PlayerCount());
      for (std::size_t i = 0; i < out.size(); i++) {
        out.at(i).first  = i;
        out.at(i).second = GetValidTurnStartCommands(i);
      }
      return out;
    }();

    const auto turn = RequestCommands(player_indices);
    if (queued_commands.size() == 0) {
      queued_commands.push_back(turn);
    } else {
      queued_commands.at(0).Merge(turn);
    }
  }

  void NextTurn() {
    if (queued_commands.size() > 0) {
      queued_commands.erase(queued_commands.begin());
    }
  }

  void EndBattle(std::vector<std::size_t> winners) {
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