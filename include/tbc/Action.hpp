#ifndef TBC_ACTION_HPP
#define TBC_ACTION_HPP

#include <functional>
#include <optional>

#include "tbc/Slot.h"
#include "tbc/Target.h"
#include "tbc/UserEffect.hpp"

namespace ngl::tbc {
template <typename TUserEffect>
class Action {
public:
  Action(Slot::Index user_, const std::vector<Target> &targets_, std::function<std::optional<TUserEffect>(void)> next)
      : user{user_}, targets{targets_}, NextEffect{next} {}
  Slot::Index user;
  std::vector<Target> targets;
  std::function<std::optional<TUserEffect>(void)> NextEffect;
};
} // namespace ngl::tbc

#endif