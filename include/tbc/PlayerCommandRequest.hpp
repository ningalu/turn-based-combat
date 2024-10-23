#ifndef TBC_PLAYERCOMMANDREQUEST_HPP
#define TBC_PLAYERCOMMANDREQUEST_HPP

namespace ngl::tbc {
template <typename TCommand>
struct PlayerCommandRequest {
  std::size_t player;
  typename TCommand::PayloadTypeSet valid_commands;
};
} // namespace ngl::tbc

#endif