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

// This isn't necessarily a variant of many types; the error type could be
// a simple enum/value or a union of error types, and the result may apply
// at the granularity of the entire response or to each Command received
// TODO: is there even any domain-agnostic data to put here or could
// PlayerComms be templated directly on some domain specific type?
template <typename TResult>
struct CommandResult {
  TResult payload;
};
} // namespace ngl::tbc

#endif