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

protected:
  std::size_t seed_;
  Layout layout_;
};
} // namespace ngl::tbc

#endif