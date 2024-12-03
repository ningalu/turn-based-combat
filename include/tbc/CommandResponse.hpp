#ifndef TBC_COMMANDERROR_HPP
#define TBC_COMMANDERROR_HPP

namespace ngl::tbc {
template <typename TCommandError>
struct CommandResponse {
  bool pass;
  std::optional<TCommandError> error;
};
} // namespace ngl::tbc

#endif