#ifndef TBC_COMMANDQUEUE_HPP
#define TBC_COMMANDQUEUE_HPP

#include <cassert>
#include <vector>

#include "util.h"

namespace ngl::tbc {
template <typename TCommand>
struct CommandQueue {
  std::vector<TCommand> dynamic_commands;
  std::vector<TCommand> static_commands;

  void BufferCommand(const TCommand &command) {
    static_commands.push_back(command);
  }
};
} // namespace ngl::tbc

#endif