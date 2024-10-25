#ifndef TBC_ACTIONABLE_HPP
#define TBC_ACTIONABLE_HPP

#include <cstdint>
#include <variant>

namespace ngl::tbc {
template <typename TCommand, typename TEvent>
using Actionable = std::variant<TCommand, std::size_t, TEvent>;
}

#endif