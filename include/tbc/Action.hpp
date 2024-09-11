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
  Action(const DeferredEffect<TBattle> &d) : Action(Deferred{d}) {}
  Action(const DeferredUserEffect<TBattle> &d) : Action(Deferred{d}) {}
  Action(const Deferred &d) : effect_{d} {}

  [[nodiscard]] std::optional<typename Effect<TBattle>::Result> ApplyNext(TBattle &b) {
    return std::visit([&](auto &&deferred) {
      return deferred.Done() ? std::nullopt : deferred.ApplyNext(b);
    },
                      effect_);
  }

protected:
  Deferred effect_;
};
} // namespace ngl::tbc

#endif