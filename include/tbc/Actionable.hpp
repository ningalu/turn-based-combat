#ifndef TBC_ACTIONABLE_HPP
#define TBC_ACTIONABLE_HPP

#include <cstdint>
#include <variant>

namespace ngl::tbc {
enum class SimultaneousActionStrategy {
  ENABLED,
  DISABLED
};

template <typename TCommand, typename TEvent, SimultaneousActionStrategy TSimultaneousActionStrategy>

struct ActionableHelper {};

template <typename TCommand, typename TEvent>
struct ActionableHelper<TCommand, TEvent, SimultaneousActionStrategy::DISABLED> {
  using ActionableImpl = std::variant<TCommand, std::size_t, TEvent>;
};

template <typename TCommand, typename TEvent>
struct ActionableHelper<TCommand, TEvent, SimultaneousActionStrategy::ENABLED> {
  using ActionableImpl = std::vector<std::variant<TCommand, std::size_t, TEvent>>;
};

template <typename TCommand, typename TEvent, SimultaneousActionStrategy TSimultaneousActionStrategy>
  requires((TSimultaneousActionStrategy == SimultaneousActionStrategy::ENABLED) || (TSimultaneousActionStrategy == SimultaneousActionStrategy::DISABLED))
using Actionable = typename ActionableHelper<TCommand, TEvent, TSimultaneousActionStrategy>::ActionableImpl;
} // namespace ngl::tbc

#endif