#ifndef TBC_LOG_H
#define TBC_LOG_H

#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace ngl::tbc {
struct Log {
  using Handler = std::function<void(const std::string &)>;

  explicit Log(std::size_t num_players);
  Log(std::size_t num_players, const std::string &default_message);

  void Insert(std::size_t player, const std::string &message);
  void Insert(const std::unordered_set<std::size_t> &players, const std::string &message);
  void Insert(std::nullopt_t spectator, const std::string &message);
  void Insert(std::optional<std::size_t> spectator, const std::string &message);
  void Insert(const std::unordered_set<std::optional<std::size_t>> &players, const std::string &message);

  [[nodiscard]] std::optional<std::string> Retrieve(std::size_t player) const;
  [[nodiscard]] std::optional<std::string> Retrieve(std::nullopt_t spectator) const;
  [[nodiscard]] std::optional<std::string> Retrieve(std::optional<std::size_t> index) const;

  void Clear(std::size_t player);
  void Clear(std::nullopt_t spectator);

  // TODO: this is dumb. whats a way to consolidate the std::string storage without duplication or pointer invalidation?
  std::unordered_map<std::optional<std::size_t>, std::string> distribution;
};
} // namespace ngl::tbc

#endif