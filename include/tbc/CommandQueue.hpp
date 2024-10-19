#ifndef TBC_COMMANDQUEUE_HPP
#define TBC_COMMANDQUEUE_HPP

#include <cassert>
#include <vector>

#include "util/util.h"

namespace ngl::tbc {
template <typename TCommand>
struct CommandQueue {
  std::vector<TCommand> dynamic_commands;
  std::vector<TCommand> static_commands;

  void BufferCommand(const TCommand &command) {
    static_commands.push_back(command);
  }

  void Merge(const CommandQueue<TCommand> &q) {
    dynamic_commands.insert(dynamic_commands.begin(), q.dynamic_commands.begin(), q.dynamic_commands.end());
    static_commands.insert(static_commands.begin(), q.static_commands.begin(), q.static_commands.end());
  }
};
} // namespace ngl::tbc

#endif