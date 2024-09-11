#ifndef TBC_BATTLE_H
#define TBC_BATTLE_H

#include <cstdint>
#include <future>
#include <vector>

#include "tbc/Command.hpp"
#include "tbc/Layout.h"
#include "tbc/PlayerComms.hpp"

namespace ngl::tbc {
template <typename TUnit, typename TState>
class Battle : public TState {
public:
  Battle(std::size_t seed, const Layout &layout) : TState{}, seed_{seed}, layout_{layout} {}
  Battle(const TState &state_, std::size_t seed, const Layout &layout) : TState{state_}, seed_{seed}, layout_{layout} {}

  void EndBattle(std::vector<std::size_t> winners) {
    winner_indices_ = winners;
  }

  [[nodiscard]] bool HasEnded() const {
    return winner_indices_ == std::nullopt;
  }

  [[nodiscard]] std::vector<std::size_t> GetWinners() const {
    return winner_indices_.value();
  }

protected:
  std::size_t seed_;
  Layout layout_;
  std::optional<std::vector<std::size_t>> winner_indices_;
};
} // namespace ngl::tbc

#endif