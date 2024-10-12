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

  [[nodiscard]] bool Done() const noexcept {
    return (dynamic_commands.size() == 0) && (static_commands.size() == 0);
  }

  [[nodiscard]] TCommand ViewNext() const {
    assert((!Done()));
    if (dynamic_commands.size() > 0) {
      return *dynamic_commands.begin();
    }
    if (static_commands.size() > 0) {
      return *static_commands.begin();
    }
    unreachable();
  }

  [[nodiscard]] TCommand TakeNext() {
    assert((!Done()));
    if (dynamic_commands.size() > 0) {
      const auto out = *dynamic_commands.begin();
      dynamic_commands.erase(dynamic_commands.begin());
      return out;
    }
    if (static_commands.size() > 0) {
      const auto out = *static_commands.begin();
      static_commands.erase(static_commands.begin());
      return out;
    }
    unreachable();
  }

  void BufferCommand(const TCommand &command) {
    static_commands.push_back(command);
  }
};
} // namespace ngl::tbc

#endif