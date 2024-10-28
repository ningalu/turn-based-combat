#include "samples/RockPaperScissors/lib/RPSTypes.h"

#include <cassert>

namespace ngl::tbc::sample::rps {

[[nodiscard]] std::optional<RPSCommand> from_str(const std::string &str) {
  if (str.size() != 1) {
    return std::nullopt;
  }

  const auto c = str.at(0);

  if ((c == 'R') || (c == 'r')) {
    return RPSCommand{Play::ROCK};
  }
  if ((c == 'P') || (c == 'p')) {
    return RPSCommand{Play::PAPER};
  }
  if ((c == 'S') || (c == 's')) {
    return RPSCommand{Play::SCISSORS};
  }

  return std::nullopt;
}

using ValidationRes = std::pair<std::optional<std::vector<CmdPayload>>, CmdResult>;
[[nodiscard]] std::pair<std::optional<std::vector<CmdPayload>>, CmdResult> ValidateCommands([[maybe_unused]] std::size_t player, const std::vector<CmdPayload> &commands, [[maybe_unused]] const Game &battle) {

  if (commands.size() != 1) {
    return ValidationRes{std::nullopt, CmdResult{false}};
  }

  const auto play = std::get<RPSCommand>(commands.front());
  assert((play.play == Play::ROCK) || (play.play == Play::PAPER) || (play.play == Play::SCISSORS));

  return ValidationRes{commands, CmdResult{true}};
}

[[nodiscard]] Schedule GenerateSchedule(Game &battle, [[maybe_unused]] const std::vector<Cmd> &buffered_commands, [[maybe_unused]] std::size_t turn) {
  const auto commands = battle.RequestCommands(std::vector<std::size_t>{0, 1});
  return Schedule{commands};
}

[[nodiscard]] Act TranslateAction([[maybe_unused]] const std::vector<Cmd> &command, [[maybe_unused]] const Game &battle_arg) {
  assert(command.size() == 2);
  Effect e{
    [=]([[maybe_unused]] Game &battle) {
      EffectResult out;
      const auto c1 = std::get<RPSCommand>(command.at(0).payload);
      const auto c2 = std::get<RPSCommand>(command.at(1).payload);
      if (c1.play == c2.play) {
        return out;
      }

      if (c1.play == Play::ROCK) {
        if (c2.play == Play::PAPER) {
          out.winners = std::vector<std::size_t>{command.at(1).player};
          return out;
        }
        if (c2.play == Play::SCISSORS) {
          out.winners = std::vector<std::size_t>{command.at(0).player};
          return out;
        }
      }

      if (c1.play == Play::PAPER) {
        if (c2.play == Play::ROCK) {
          out.winners = std::vector<std::size_t>{command.at(0).player};
          return out;
        }
        if (c2.play == Play::SCISSORS) {
          out.winners = std::vector<std::size_t>{command.at(1).player};
          return out;
        }
      }

      if (c1.play == Play::SCISSORS) {
        if (c2.play == Play::ROCK) {
          out.winners = std::vector<std::size_t>{command.at(1).player};
          return out;
        }
        if (c2.play == Play::PAPER) {
          out.winners = std::vector<std::size_t>{command.at(0).player};
          return out;
        }
      }

      return EffectResult{};
    }
  };
  return Act{e};
}

} // namespace ngl::tbc::sample::rps