#include <iostream>
#include <memory>
#include <string>
#include <variant>

#include "tbc/Battle.hpp"
#include "tbc/Command.hpp"
#include "tbc/PlayerComms.hpp"

#include "tbc/lib.hpp"

using MyCommands = ngl::tbc::Command<int, double>;
auto main() -> int {
  std::cout << "ningalu tbc\n";

  auto p1 = std::make_unique<ngl::tbc::PlayerComms<MyCommands>>("Player 1", []() { return std::vector<MyCommands>({MyCommands{1}}); }, [](std::size_t) { return MyCommands(1.0); });
  auto p2 = std::make_unique<ngl::tbc::PlayerComms<MyCommands>>("Player 2", []() { return std::vector<MyCommands>({MyCommands{2.0}}); }, [](std::size_t) { return MyCommands(1); });
  std::vector<std::unique_ptr<ngl::tbc::PlayerComms<MyCommands>>> players;
  players.emplace_back(std::move(p1));
  players.emplace_back(std::move(p2));
  auto b = ngl::tbc::Battle<int, MyCommands>{std::move(players)};

  for (const auto &r : b.RequestStaticCommands()) {
    std::visit([](const auto &p) { std::cout << p << "\n"; }, r.payload);
  }
  return 0;
}
