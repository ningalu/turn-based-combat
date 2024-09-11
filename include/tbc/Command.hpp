#ifndef TBC_COMMAND_HPP
#define TBC_COMMAND_HPP

#include <variant>

namespace ngl::tbc {
template <typename... Ts>
struct Command {
  using Payload = std::variant<Ts...>;

  Command(std::size_t player_, std::variant<Ts...> p) : player{player_}, payload{p} {}
  std::size_t player;
  Payload payload;
};
} // namespace ngl::tbc

#endif