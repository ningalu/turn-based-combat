#ifndef TBC_COMMS_HPP
#define TBC_COMMS_HPP

#include <vector>

#include "tbc/Log.h"
#include "tbc/PlayerComms.hpp"

namespace ngl::tbc {
template <typename TCommand, typename TCommandResult>
struct Comms {
  using TPlayerComms = PlayerComms<TCommand, TCommandResult>;
  // std::queue? any reason to look at past logs?
  std::vector<Log> log_buffer_;
  Log::Handler master_log_output_;
  std::vector<TPlayerComms> players;
  Log::Handler spectator_log_output_;

  Comms(std::vector<TPlayerComms> players_) : players{std::move(players_)} {}

  [[nodiscard]] std::size_t PlayerCount() const {
    return players.size();
  }

  void SetSpectatorLogHandler(Log::Handler handler) { spectator_log_output_ = handler; }

  void SetPlayerLogHandler(std::size_t player, Log::Handler handler) {
    if (player >= players.size()) {
      throw std::out_of_range{"Assigning Log handler to invalid player index: " + std::to_string(player)};
    }
    players.at(player).SetLogHandler(handler);
  }

  void SetMasterLogHandler(Log::Handler handler) { master_log_output_ = handler; }

  void PostLog(const std::string &message) {
    log_buffer_.push_back(Log{players.size(), message});
  }

  void PostLog(std::optional<std::size_t> player, const std::string &message) {
    Log log{players.size()};
    log.Insert(player, message);
    log_buffer_.push_back(log);
  }

  void PostLog(const std::unordered_set<std::optional<std::size_t>> &players, const std::string &message) {
    Log log{players.size()};
    log.Insert(players, message);
    log_buffer_.push_back(log);
  }

  void PostLog(const Log &log) {
    log_buffer_.push_back(log);
  }

  void Flush() {
    // TODO: post logs asynchronously
    for (const auto &log : log_buffer_) {
      for (std::size_t i = 0; i < players.size(); i++) {
        const auto message = log.Retrieve(i);
        if (message.has_value()) {
          players.at(i).PostLog(message.value());
        }
      }
      const auto spectator_message = log.Retrieve(std::nullopt);
      if (spectator_message.has_value()) {
        spectator_log_output_(spectator_message.value());
      }
      log_buffer_.erase(log_buffer_.begin());
    }
  }
};
} // namespace ngl::tbc

#endif