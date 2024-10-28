#ifndef TBC_ACTION_HPP
#define TBC_ACTION_HPP

#include <functional>
#include <optional>
#include <variant>

#include "tbc/Effect.hpp"

namespace ngl::tbc {
template <typename TBattle, typename TCommand, typename TEvent>
class Action {
  using TEffect          = Effect<TBattle, TCommand, TEvent>;
  using TAction          = Action<TBattle, TCommand, TEvent>;
  using TransferFunction = typename TEffect::TransferFunction;

public:
  using Result = typename Effect<TBattle, TCommand, TEvent>::Result;

protected:
  using ActionImpl = std::function<std::optional<Result>(TBattle &)>;

public:
  TransferFunction pre;
  TransferFunction post;

  explicit Action(const TEffect &effects) : TAction(std::vector<TEffect>{effects}) {}
  explicit Action(const std::vector<TEffect> &effects) : TAction(EffectsImpl(effects)) {}
  explicit Action(ActionImpl impl) : started_{false}, finished_{false}, cancelled_{false}, action_impl_{std::move(impl)} {}

  [[nodiscard]] static std::function<std::optional<Result>(TBattle &)> EffectsImpl(std::vector<TEffect> effects) {
    return [effects](TBattle &battle) mutable -> std::optional<Result> {
      if (!effects.empty()) {
        Result out = effects.front().Apply(battle);
        effects.erase(effects.begin());
        return out;
      }

      return std::nullopt;
    };
  }

  [[nodiscard]] std::optional<Result> ApplyNext(TBattle &battle) {
    assert(action_impl_);

    if (cancelled_) {
      return std::nullopt;
    }

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