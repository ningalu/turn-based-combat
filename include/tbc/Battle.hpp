#ifndef TBC_BATTLE_H
#define TBC_BATTLE_H

#include <cstdint>
#include <future>
#include <vector>

#include "tbc/Command.hpp"
#include "tbc/PlayerComms.hpp"

namespace ngl::tbc {
template <typename TUnit, typename TCommand>
class Battle {
public:
  Battle(std::vector<std::unique_ptr<PlayerComms<TCommand>>> players) : players_{std::move(players)} {}

  [[nodiscard]] std::vector<TCommand> RequestStaticCommands() {
    std::vector<std::future<std::vector<TCommand>>> action_handles;

    for (std::size_t i = 0; i < players_.size(); i++) {
      // Async poll each player for static commands, rejecting and repolling if any are invalid
      action_handles.push_back(std::async(std::launch::async, [=, this]() {
        std::vector<TCommand> s;
        do {
          s = players_.at(i)->GetStaticCommand().get();
        } while (!std::all_of(s.begin(), s.end(), [=, this](const TCommand &) { return []() { return true; }(); }));

        return s;
      }));
    }

    std::vector<TCommand> actions;
    for (auto &response : action_handles) {
      auto val = response.get();
      actions.insert(actions.end(), val.begin(), val.end());
    }
    return actions;
  }

  [[nodiscard]] TCommand RequestDynamicCommand(std::size_t slot) {

    std::future<TCommand> handle;

    handle = std::async(std::launch::async, [=, this]() {
      TCommand s;
      do {
        s = players_.at(slot /*TODO: UPDATE THIS WHEN LAYOUT IS IMPLEMENTED*/)->GetDynamicCommand(slot).get();
      } while ([]() { return false; }());

      return s;
    });

    return handle.get();
  }

protected:
  std::vector<std::unique_ptr<PlayerComms<TCommand>>> players_;
};
} // namespace ngl::tbc

#endif