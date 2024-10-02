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
template <typename TBattle, typename TEvents, typename TCommands>
class Action {
  using Deferred = std::variant<DeferredEffect<TBattle, TEvents, TCommands>, DeferredUserEffect<TBattle, TEvents, TCommands>>;

  template <typename T>
  [[nodiscard]] static std::vector<Deferred> DeferredVec(const std::vector<T> &dv) {
    static_assert(std::is_same_v<T, DeferredEffect<TBattle, TEvents, TCommands>> || std::is_same_v<T, DeferredUserEffect<TBattle, TEvents, TCommands>>);
    std::vector<Deferred> out;
    for (const auto &e : dv) {
      out.push_back(Deferred{e});
    }
    return out;
  }

public:
  using Result = Effect<TBattle, TEvents, TCommands>::Result;

  Action(const std::vector<DeferredEffect<TBattle, TEvents, TCommands>> &d) : Action(DeferredVec(d)) {}
  Action(const std::vector<DeferredUserEffect<TBattle, TEvents, TCommands>> &d) : Action(DeferredVec(d)) {}
  Action(const std::vector<Deferred> &d) : effects_{d}, started_{false} {}

  [[nodiscard]] std::optional<Result> ApplyNext(TBattle &b) {
    if (!started_) {
      started_ = true;
    }

    std::optional<Result> out;
    if (!Done()) {
      out = std::visit([&](auto &&deferred) {
        return deferred.Apply(b);
      },
                       *effects_.begin());
      effects_.erase(effects_.begin());
    }
    return out;
  }

  [[nodiscard]] bool Done() {
    return !(effects_.size() > 0);
  }

  [[nodiscard]] bool Started() {
    return started_;
  }

protected:
  std::vector<Deferred> effects_;

  bool started_;
};
} // namespace ngl::tbc

#endif