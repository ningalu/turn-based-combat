#include <iostream>
#include <memory>
#include <string>
#include <variant>

#include "tbc/Battle.hpp"
#include "tbc/BattleScheduler.hpp"
#include "tbc/Command.hpp"
#include "tbc/Layout.h"
#include "tbc/PlayerComms.hpp"

#include "tbc/Layout.h"

using MyCommands = ngl::tbc::Command<int, double>;
struct MyBattleState {
  bool ended = false;
};
auto main() -> int {
  std::cout << "ningalu tbc\n";

  auto p1 = std::make_unique<ngl::tbc::PlayerComms<MyCommands>>("Player 1", []() { return std::vector<MyCommands>({MyCommands{1}}); }, [](std::size_t) { return MyCommands(1.0); });
  auto p2 = std::make_unique<ngl::tbc::PlayerComms<MyCommands>>("Player 2", []() { return std::vector<MyCommands>({MyCommands{2.0}}); }, [](std::size_t) { return MyCommands(1); });
  std::vector<std::unique_ptr<ngl::tbc::PlayerComms<MyCommands>>> players;
  players.emplace_back(std::move(p1));
  players.emplace_back(std::move(p2));

  auto l = ngl::tbc::Layout{{{{0}}, {{1}}}};

  auto b  = ngl::tbc::Battle<int, MyBattleState>{0, l};
  auto bs = ngl::tbc::BattleScheduler<MyCommands, ngl::tbc::Battle<int, MyBattleState>>{std::move(players)};

  bs.SetCommandValidator([](std::size_t, std::vector<MyCommands>) { return true; });

  for (const auto &r : bs.RequestCommands({0, 1})) {
    std::visit([](const auto &p) { std::cout << p << "\n"; }, r.payload);
  }
  return 0;
}
