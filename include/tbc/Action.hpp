#ifndef TBC_ACTION_HPP
#define TBC_ACTION_HPP

#include <functional>
#include <optional>
#include <variant>

#include "tbc/DeferredEffect.hpp"
#include "tbc/DeferredUserEffect.hpp"
#include "tbc/Slot.h"
#include "tbc/Target.h"
#include "tbc/UserEffect.hpp"

namespace ngl::tbc {
template <typename TBattle>
class Action {
  using Deferred = std::variant<DeferredEffect<TBattle>, DeferredUserEffect<TBattle>>;

public:
  using Result = Effect<TBattle>::Result;

  Action(const DeferredEffect<TBattle> &d) : Action(Deferred{d}) {}
  Action(const DeferredUserEffect<TBattle> &d) : Action(Deferred{d}) {}
  Action(const Deferred &d) : effect_{d}, started_{false} {}

  [[nodiscard]] std::optional<typename Effect<TBattle>::Result> ApplyNext(TBattle &b) {
    if (!started_) {
      started_ = true;
    }

    return std::visit([&](auto &&deferred) {
      return deferred.Done() ? std::nullopt : deferred.ApplyNext(b);
    },
                      effect_);
  }

  [[nodiscard]] bool Done() {
    return std::visit([&](auto &&deferred) {
      return deferred.Done();
    },
                      effect_);
  }

  [[nodiscard]] bool Started() {
    return started_;
  }

protected:
  Deferred effect_;

  bool started_;
};
} // namespace ngl::tbc

#endif