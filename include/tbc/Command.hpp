#ifndef TBC_COMMAND_HPP
#define TBC_COMMAND_HPP

#include <variant>

#include "util/tmp.hpp"
namespace ngl::tbc {
template <typename... TCommands>
struct Command {
  using Payload        = std::variant<TCommands...>;
  using PayloadTypeSet = tmp::typeset<TCommands...>;

  Command(std::size_t player_, std::variant<TCommands...> p) : player{player_}, payload{p} {}
  std::size_t player;
  Payload payload;
};

} // namespace ngl::tbc

#endif