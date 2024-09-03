#ifndef TBC_EFFECT_HPP
#define TBC_EFFECT_HPP

#include <vector>

#include "tbc/Battle.hpp"
#include "tbc/Slot.h"
#include "tbc/Target.h"

namespace ngl::tbc {
template <typename TBattle>
class Effect {
public:
  struct Success {};
  struct EndBattle {
    std::vector<std::size_t> winners;
  } struct RequestCommands {
    std::vector<std::size_t> players;
  };
  using Result = std::variant<Success, EndBattle, RequestCommands>;

  Effect(std::function<Result(TBattle &, const std::vector<Target> &)> f) : xfer_{f} {}

  Result Apply(TBattle &battle, const std::vector<Target> &targets) {
    return CheckBattle ? CheckBattle(battle, xfer_(battle, targets)) : xfer_(battle, targets);
  }

  static std::function<Result(const TBattle &, const Result &)> CheckBattle{};

private:
  std::function<void(TBattle &, const std::vector<Target> &)> xfer_;
};
} // namespace ngl::tbc

#endif