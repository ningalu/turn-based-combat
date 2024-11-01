#include "tbc/Battle.hpp"

namespace ngl::tbc {

Log::Log(std::size_t num_players) {
  distribution.insert({std::nullopt, nullptr});
  for (std::size_t i = 0; i < num_players; i++) {
    distribution.insert({std::optional{i}, nullptr});
  }
}
Log::Log(std::size_t num_players, const std::string &default_message) {
  messages.insert(default_message);
  const auto *addr = &messages.extract(default_message).value();

  distribution.insert({std::nullopt, addr});
  for (std::size_t i = 0; i < num_players; i++) {
    distribution.insert({std::optional{i}, addr});
  }
}

void Log::Insert(std::size_t player, const std::string &message) {
  if (messages.contains(message)) {
    const auto *addr     = &messages.extract(message).value();
    distribution[player] = addr;
  }
}

void Log::Insert(const std::unordered_set<std::size_t> &players, const std::string &message) {
  for (const auto player : players) {
    Insert(player, message);
  }
}

void Log::Insert(std::nullopt_t spectator, const std::string &message) {
  if (messages.contains(message)) {
    const auto *addr        = &messages.extract(message).value();
    distribution[spectator] = addr;
  }
}

void Log::Insert(std::optional<std::size_t> spectator, const std::string &message) {
  if (messages.contains(message)) {
    const auto *addr        = &messages.extract(message).value();
    distribution[spectator] = addr;
  }
}

void Log::Insert(const std::unordered_set<std::optional<std::size_t>> &players, const std::string &message) {
  for (const auto player : players) {
    if (player.has_value()) {
      Insert(player.value(), message);
    } else {
      Insert(std::nullopt, message);
    }
  }
}

[[nodiscard]] std::optional<const std::string *> Log::Retrieve(std::size_t player) const {
  assert(distribution.contains(std::optional{player}));
  using TOut      = std::optional<const std::string *>;
  const auto *ptr = distribution.at(player);
  return (ptr == nullptr) ? TOut{std::nullopt} : TOut{ptr};
}

[[nodiscard]] std::optional<const std::string *> Log::Retrieve(std::nullopt_t spectator) const {
  assert(distribution.contains(spectator));
  using TOut      = std::optional<const std::string *>;
  const auto *ptr = distribution.at(spectator);
  return (ptr == nullptr) ? TOut{std::nullopt} : TOut{ptr};
}

} // namespace ngl::tbc