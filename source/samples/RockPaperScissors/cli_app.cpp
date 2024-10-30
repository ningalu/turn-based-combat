#include <cassert>
#include <iostream>

#include "samples/RockPaperScissors/lib/RPSTypes.h"

#include "util/util.h"

using namespace ngl::tbc::sample::rps;

int main([[maybe_unused]] int argc, [[maybe_unused]] char **argv) {
  srand(static_cast<unsigned int>(time(NULL)));
  Player player{
    "Player",
    [&]([[maybe_unused]] auto &&...whatever) -> std::vector<CmdPayload> {
      std::string in;
      std::optional<RPSCommand> command;
      while (!command.has_value()) {
        std::cout << "Please enter a play with one of R | P | S\n";
        std::cin >> in;
        command = from_str(in);
      }
      return std::vector{CmdPayload{command.value()}};
    }
  };

  Player cpu{
    "CPU",
    []([[maybe_unused]] auto &&...whatever) -> std::vector<CmdPayload> {
      std::vector<CmdPayload> out;
      switch (rand() % 3) {
      case 0:
        out = std::vector{CmdPayload{RPSCommand{Play::ROCK}}};
        return out;
      case 1:
        out = std::vector{CmdPayload{RPSCommand{Play::PAPER}}};
        return out;
      case 2:
        out = std::vector{CmdPayload{RPSCommand{Play::SCISSORS}}};
        return out;
      default:
        ngl::tbc::unreachable();
      }
    }
  };

  Game g{{player, cpu}};
  g.SetCommandValidator(ValidateCommands);

  Scheduler s;
  s.SetScheduleGenerator(GenerateSchedule);
  s.SetActionTranslator(TranslateAction);

  const auto winner = s.RunBattle(g);
  assert(winner.size() == 1);
  std::cout << "Winner: Player " << winner.front() << "\n";
  return 0;
}