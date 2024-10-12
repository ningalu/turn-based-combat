#ifndef TBC_BATTLE_H
#define TBC_BATTLE_H

#include <cassert>
#include <cstdint>
#include <future>
#include <vector>

#include "tbc/Command.hpp"
#include "tbc/Layout.h"
#include "tbc/PlayerComms.hpp"

namespace ngl::tbc {
template <typename TUnit, typename TState, typename TCommand>
class Battle : public TState {
  using TCommandPayload   = typename TCommand::Payload;
  using TCommandValidator = std::function<std::optional<std::vector<TCommandPayload>>(std::size_t, std::vector<TCommandPayload>, const Battle<TUnit, TState, TCommand> &)>;

public:
  Battle(const TState &state_, std::size_t seed, const std::vector<PlayerComms<TCommandPayload>> &comms, const Layout &layout) : TState{state_}, seed_{seed}, comms_{comms}, layout_{layout} {}
  Battle(std::size_t seed, const std::vector<PlayerComms<TCommandPayload>> &comms, const Layout &layout) : Battle{{}, seed, comms, layout} {}

  std::vector<std::vector<TCommand>> queued_commands;

  [[nodiscard]] const Layout &layout() const {
    return layout_;
  }

  [[nodiscard]] Slot GetSlot(Slot::Index i) const {
    return layout_.GetSlot(i);
  }

  [[nodiscard]] std::size_t PlayerCount() const {
    return comms_.size();
  }

  void SetCommandValidator(const TCommandValidator &validator) {
    command_validator_ = validator;
  }

  // TODO: is there actually any meaningful domain-agnostic validation that can be done here?
  [[nodiscard]] std::optional<std::vector<TCommandPayload>> ValidateCommands(std::size_t authority, const std::vector<TCommandPayload> &commands) const {
    if (command_validator_) {
      return command_validator_(authority, commands, *this);
    }
    // TODO: deal with buffered commands somehow
    return commands;
  }

  [[nodiscard]] std::vector<TCommand> RequestCommands(const std::vector<std::size_t> &players, std::size_t attempts) {
    return RequestCommands(players, nullptr, attempts);
  }

  [[nodiscard]] std::vector<TCommand> RequestCommands(const std::vector<std::size_t> &players, TCommandValidator validator = nullptr, std::size_t attempts = 10) {

    std::vector<std::future<std::vector<TCommand>>> action_handles;

    for (const auto player : players) {
      assert(comms_.size() > player);

      // Async poll each player for static commands, rejecting and repolling if any are invalid
      action_handles.push_back(std::async(std::launch::async, [=, this]() {
        std::optional<std::vector<TCommandPayload>> payloads;
        for (std::size_t i = 0; i < attempts; i++) {
          const auto incoming_payloads = comms_.at(player).GetCommands().get();
          payloads                     = (validator ? validator : command_validator_)(player, incoming_payloads, *this);
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
    while (queued_commands.size() < (turns_ahead + 1)) {
      queued_commands.push_back({});
    }

    queued_commands.at(turns_ahead).push_back(c);
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
  std::vector<PlayerComms<TCommandPayload>> comms_;
  Layout layout_;
  // TODO: refactor
  std::optional<std::vector<std::size_t>> winner_indices_;

  TCommandValidator command_validator_;
};
} // namespace ngl::tbc

#endif