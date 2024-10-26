#include <charconv>
#include <iostream>
#include <stdlib.h>
#include <string>
#include <string_view>
#include <vector>

#include "samples/NoughtsAndCrosses/lib/NACTypes.h"
#include "util/util.h"

using namespace ngl::tbc::sample::nac;

[[nodiscard]] std::optional<NACCommand> string_to_coord(const std::string &in) {
  const auto parts = ngl::tbc::split(std::string_view{in}, ',');
  if (parts.size() != 2) {
    return std::nullopt;
  }

  NACCommand out;
  for (auto const [addr, index] : std::vector<std::pair<uint8_t *, std::size_t>>{{&out.x, 0U}, {&out.y, 1U}}) {
    int val;
    auto [ptr, ec] = std::from_chars(parts.at(index).data(), parts.at(index).data() + parts.at(index).size(), val);
    if (ec != std::errc()) {
      return std::nullopt;
    }
    if ((val < 0) || (val > 2)) {
      return std::nullopt;
    }
    *addr = static_cast<std::uint8_t>(val);
  }

  return out;
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char **argv) {
  // TODO: better helper rand facilities
  srand(static_cast<unsigned int>(time(NULL)));

  NACTypes::TPlayerComms player{
    "Player",
    [&]([[maybe_unused]] auto &&whatever) -> std::vector<NACTypes::TCommandPayload> {
      std::string in;
      std::vector<std::string_view> parts;
      std::optional<NACCommand> command;
      while (!command.has_value()) {
        std::cout << "Please enter a move in the format \"x,y\"\n";
        std::cin >> in;
        command = string_to_coord(in);
      }
      return std::vector{NACTypes::TCommandPayload{command.value()}};
    }
  };
  player.SetLogHandler([](const std::string &log) {
    std::cout << log << "\n";
  });

  NACTypes::TPlayerComms cpu{
    "CPU",
    [&]([[maybe_unused]] auto &&whatever) -> std::vector<NACTypes::TCommandPayload> {
      NACCommand command;
      command.x = static_cast<uint8_t>(rand() % 3);
      command.y = static_cast<uint8_t>(rand() % 3);
      return std::vector{NACTypes::TCommandPayload{command}};
    }
  };

  Game game{
    NACState{},
    std::vector<NACTypes::TPlayerComms>{player, cpu}
  };
  game.SetCommandValidator(ValidateCommands);

  Scheduler scheduler;
  scheduler.SetScheduleGenerator(GenerateSchedule);
  scheduler.SetActionTranslator(TranslateAction);
  scheduler.SetHandler<ngl::tbc::DefaultEvents::PlannedActionEnd>(ActionEndHandler);

  // OOX
  // XXO
  // OX
  // game.state.board[0][0] = NACPlayer::NOUGHT;
  // game.state.board[0][1] = NACPlayer::NOUGHT;
  // game.state.board[0][2] = NACPlayer::CROSS;
  // game.state.board[1][0] = NACPlayer::CROSS;
  // game.state.board[1][1] = NACPlayer::CROSS;
  // game.state.board[1][2] = NACPlayer::NOUGHT;
  // game.state.board[2][0] = NACPlayer::NOUGHT;
  // game.state.board[2][1] = NACPlayer::CROSS;

  const auto winners = scheduler.RunBattle(game);
  assert(!(winners.size() > 1));
  if (winners.size() == 0) {
    std::cout << "Tie\n";
  } else {
    std::cout << "Player " << winners.at(0) << " wins\n";
  }
  return 0;
}