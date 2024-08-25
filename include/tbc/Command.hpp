#ifndef TBC_COMMAND_HPP
#define TBC_COMMAND_HPP

#include <variant>

namespace ngl::tbc {
template <typename... T>
struct Command {
  Command(std::variant<T...> p) : payload{p} {}
  std::variant<T...> payload;
};
} // namespace ngl::tbc

#endif