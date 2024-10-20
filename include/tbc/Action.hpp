#ifndef TBC_ACTION_HPP
#define TBC_ACTION_HPP

#include <functional>
#include <optional>
#include <variant>

#include "tbc/DeferredEffect.hpp"
#include "tbc/DeferredUserEffect.hpp"
#include "tbc/Slot.h"
#include "tbc/Target.hpp"
#include "tbc/UserEffect.hpp"

namespace ngl::tbc {
template <typename TBattle, typename TEvents, typename TCommands>
class Action {
public:
  using Result = Effect<TBattle, TEvents, TCommands>::Result;

protected:
  using Deferred   = std::variant<DeferredEffect<TBattle, TEvents, TCommands>, DeferredUserEffect<TBattle, TEvents, TCommands>>;
  using TDecorator = std::function<Result(TBattle &)>;

  template <typename T>
  [[nodiscard]] static std::vector<Deferred> DeferredVec(const std::vector<T> &dv) {
    static_assert(std::is_same_v<T, DeferredEffect<TBattle, TEvents, TCommands>> || std::is_same_v<T, DeferredUserEffect<TBattle, TEvents, TCommands>>);
    std::vector<Deferred> out;
    for (const auto &e : dv) {
      out.push_back(Deferred{e});
    }
    return out;
  }

  using ActionImpl = std::function<std::optional<Result>(TBattle &)>;

public:
  TDecorator pre;
  TDecorator post;

  Action(const std::vector<DeferredEffect<TBattle, TEvents, TCommands>> &d) : Action(DeferredVec(d)) {}
  Action(const std::vector<DeferredUserEffect<TBattle, TEvents, TCommands>> &d) : Action(DeferredVec(d)) {}
  Action(const std::vector<Deferred> &d) : Action<TBattle, TEvents, TCommands>(DeferredEffectsImpl(d)) {}
  Action(const ActionImpl &impl) : started_{false}, finished_{false}, cancelled_{false}, action_impl_{impl} {}

  // TODO: should this be taking a copy? i only really want the lambda capture to copy this
  [[nodiscard]] static std::function<std::optional<Result>(TBattle &)> DeferredEffectsImpl(std::vector<Deferred> deferred) {
    return [=](TBattle &battle) mutable -> std::optional<Result> {
      std::optional<Result> out;
      if (deferred.size() > 0) {
        out = std::visit([&battle](auto &&payload) {
          return payload.Apply(battle);
        },
                         deferred.front());
        deferred.erase(deferred.begin());
        return out;
      } else {
        return std::nullopt;
      }
    };
  }

  [[nodiscard]] std::optional<Result> ApplyNext(TBattle &battle) {
    assert(action_impl_);

    if (!started_) {
      started_ = true;

      if (pre) {
        return pre(battle);
      }
    }

    const auto res = action_impl_(battle);
    if (res.has_value()) {
      return res.value();
    }

    // main action impl is finished
    if (!finished_) {
      finished_ = true;
      if (post) {
        return post(battle);
      }
    }

    return std::nullopt;
  }

  [[nodiscard]] bool Started() {
    return started_;
  }

  void Cancel() {
    cancelled_ = true;
  }

protected:
  bool started_;
  bool finished_;
  bool cancelled_;
  std::function<std::optional<Result>(TBattle &)> action_impl_;
};
} // namespace ngl::tbc

#endif