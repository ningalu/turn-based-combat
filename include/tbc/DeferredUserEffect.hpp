#ifndef TBC_DEFERREDUSEREFFECT_HPP
#define TBC_DEFERREDUSEREFFECT_HPP

#include <optional>

#include "tbc/Battle.hpp"
#include "tbc/Slot.h"
#include "tbc/Target.h"
#include "tbc/UserEffect.hpp"

namespace ngl::tbc {
template <typename TBattle>
class DeferredUserEffect {
  using Result = UserEffect<TBattle>::Result;

public:
  DeferredUserEffect(Slot::Index user, const std::vector<UserEffect<TBattle>> &effects, const std::vector<Target> &targets) : user_{user}, effects_{effects}, targets_{targets} {}
  [[nodiscard]] std::optional<Result> ApplyNext(TBattle &b) {
    std::optional<Result> out = std::nullopt;
    if (effects_.size() > 0) {
      out = effects_.at(0).Apply(user_, b, targets_);
      effects_.erase(effects_.begin());
    }
    return out;
  }

  [[nodiscard]] bool Done() const {
    return !(effects_.size() > 0);
  }

protected:
  Slot::Index user_;
  std::vector<UserEffect<TBattle>> effects_;
  std::vector<Target> targets_;
};
} // namespace ngl::tbc

#endif