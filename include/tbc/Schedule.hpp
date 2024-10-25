#ifndef TBC_SCHEDULE_HPP
#define TBC_SCHEDULE_HPP

#include <vector>

#include "tbc/Actionable.hpp"

namespace ngl::tbc {
template <typename TCommand, typename TEvent>
struct Schedule {
  using TActionable = Actionable<TCommand, TEvent>;

  std::vector<std::vector<TActionable>> order;
  Schedule() = default;

  Schedule(std::vector<TCommand> commands) {
    for (const auto &command : commands) {
      order.push_back(std::vector<TActionable>{TActionable{command}});
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