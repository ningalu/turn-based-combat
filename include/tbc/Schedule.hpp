#ifndef TBC_SCHEDULE_HPP
#define TBC_SCHEDULE_HPP

#include <vector>

#include "tbc/Actionable.hpp"

namespace ngl::tbc {
template <typename TCommand, typename TEvent, SimultaneousActionStrategy TSimultaneousActionStrategy>
struct Schedule {
  using TActionable = Actionable<TCommand, TEvent, TSimultaneousActionStrategy>;

  std::vector<TActionable> order;
  Schedule() = default;

  explicit Schedule(const std::vector<TCommand> &commands) {
    if constexpr (TSimultaneousActionStrategy == SimultaneousActionStrategy::DISABLED) {
      for (const auto &command : commands) {
        order.push_back(TActionable{command});
      }
    } else {
      TActionable action;
      for (const auto command : commands) {
        action.push_back({command});
      }
      order.push_back(action);
    }
  }

  explicit Schedule(const std::vector<std::size_t> &players) {
    if constexpr (TSimultaneousActionStrategy == SimultaneousActionStrategy::DISABLED) {
      for (const auto &player : players) {
        order.push_back(TActionable{player});
      }
    } else {
      TActionable action;
      for (const auto player : players) {
        action.push_back({player});
      }
      order.push_back(action);
    }
  }

  [[nodiscard]] bool Empty() const { return order.empty(); }

  void Next() {
    assert(!Empty());
    order.erase(order.begin());
  }

  [[nodiscard]] TActionable Take() {
    assert(!Empty());
    const auto out = *order.begin();
    Next();
    return out;
  }
};
} // namespace ngl::tbc

#endif