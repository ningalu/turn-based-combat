#ifndef TBC_COMMAND_HPP
#define TBC_COMMAND_HPP

#include <variant>

namespace ngl::tbc {
template <typename... T>
struct Command {
  using Payload = std::variant<T...>;

  Command(std::variant<T...> p) : payload{p} {}
  Payload payload;
};
} // namespace ngl::tbc

#endif