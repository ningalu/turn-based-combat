#include "tbc/Log.h"

#include <cassert>

namespace ngl::tbc {

Log::Log(std::size_t num_players) {
  distribution.insert({std::nullopt, nullptr});
  for (std::size_t i = 0; i < num_players; i++) {
    distribution.insert({std::optional{i}, nullptr});
  }
}
Log::Log(std::size_t num_players, const std::string &default_message) {
  distribution.insert({std::nullopt, default_message});
  for (std::size_t i = 0; i < num_players; i++) {
    distribution.insert({std::optional{i}, default_message});
  }
}

void Log::Insert(std::size_t player, const std::string &message) { distribution[player] = message; }

void Log::Insert(const std::unordered_set<std::size_t> &players, const std::string &message) {
  for (const auto player : players) {
    Insert(player, message);
  }
}

void Log::Insert(std::nullopt_t spectator, const std::string &message) { distribution[spectator] = message; }
void Log::Insert(std::optional<std::size_t> index, const std::string &message) { distribution[index] = message; }

void Log::Insert(const std::unordered_set<std::optional<std::size_t>> &players, const std::string &message) {
  for (const auto player : players) {
    Insert(player, message);
  }
}

[[nodiscard]] std::optional<std::string> Log::Retrieve(std::size_t player) const {
  if (distribution.contains(std::optional{player})) {
    return distribution.at(player);
  } else {
    return std::nullopt;
  }
}

[[nodiscard]] std::optional<std::string> Log::Retrieve(std::nullopt_t spectator) const {
  if (distribution.contains(spectator)) {
    return distribution.at(spectator);
  } else {
    return std::nullopt;
  }
}

[[nodiscard]] std::optional<std::string> Log::Retrieve(std::optional<std::size_t> index) const {
  if (index.has_value()) {
    return Retrieve(index.value());
  } else {
    return Retrieve(std::nullopt);
  }
}

} // namespace ngl::tbc