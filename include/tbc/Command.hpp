#ifndef TBC_COMMAND_HPP
#define TBC_COMMAND_HPP

#include <variant>

#include "util/tmp.hpp"
namespace ngl::tbc {
template <typename... TTypes>
struct Command {
  using Payload        = std::variant<TTypes...>;
  using PayloadTypeSet = tmp::typeset<TTypes...>;

  Command(std::size_t player_, std::variant<TTypes...> p) : player{player_}, payload{p} {}
  std::size_t player;
  Payload payload;
};
} // namespace ngl::tbc

#endif