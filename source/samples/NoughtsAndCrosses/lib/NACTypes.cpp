#include "samples/NoughtsAndCrosses/lib/NACTypes.h"

#include <charconv>
#include <stdexcept>

#include "util/util.h"

namespace ngl::tbc::sample::nac {

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

[[nodiscard]] char str(NACPlayer p) {
  switch (p) {
  case NACPlayer::NOUGHT:
    return 'O';
  case NACPlayer::CROSS:
    return 'X';
  default:
    throw std::logic_error{"Invalid player"};
  }
}

[[nodiscard]] std::string NACState::str() const {
  std::string out;
  for (const auto &row : board) {
    std::string row_str;
    for (const auto &tile : row) {
      if (tile.has_value()) {
        row_str.push_back(ngl::tbc::sample::nac::str(tile.value()));
        row_str.push_back(' ');
      } else {
        row_str.push_back(' ');
        row_str.push_back(' ');
      }
    }
    out.append(row_str);
    out.push_back('\n');
  }
  return out;
}

[[nodiscard]] std::pair<std::optional<std::vector<CommandPayload>>, CommandResult> ValidateCommands([[maybe_unused]] std::size_t player, const std::vector<CommandPayload> &commands, const Game &battle) {
  if (commands.size() != 1) {
    return std::pair{std::optional<std::vector<CommandPayload>>{std::nullopt}, CommandResult{false}};
  }

  const auto &[x, y] = std::get<NACCommand>(commands.at(0));
  if (battle.state.board[x][y] != std::nullopt) {
    return std::pair{std::optional<std::vector<CommandPayload>>{std::nullopt}, CommandResult{false}};
  }
  return std::pair{std::optional<std::vector<CommandPayload>>{std::vector{commands.at(0)}}, CommandResult{true}};
}

[[nodiscard]] Schedule GenerateSchedule([[maybe_unused]] Game &battle, [[maybe_unused]] const std::vector<Command> &buffered_commands, [[maybe_unused]] std::size_t turn) {
  return std::vector<std::size_t>{0, 1};
}

[[nodiscard]] Action TranslateAction(const Command &command, [[maybe_unused]] const Game &battle_arg) {
  // TODO: refactor effects so you dont haev to rig this shit up
  int run = 0;
  return Action{[command, run](Game &battle) mutable -> std::optional<EffectResult> {
    if (run == 0) {
      run++;
      assert((command.player == 0) || (command.player == 1));
      const auto [x, y] = std::get<NACCommand>(command.payload);
      assert(x < 3);
      assert(y < 3);
      assert(battle.state.board[x][y] == std::nullopt);
      battle.state.board[x][y] = (command.player == 0) ? NACPlayer::NOUGHT : NACPlayer::CROSS;
      battle.PostLog(battle.state.str());
      return EffectResult{};
    } else {
      return std::nullopt;
    }
  }};
}

[[nodiscard]] std::vector<Action> ActionEndHandler([[maybe_unused]] ngl::tbc::DefaultEvents::PlannedActionEnd e, [[maybe_unused]] Game &battle_arg) {
  int i = 0;
  return std::vector{
    Action{
      [i](Game &battle) mutable -> std::optional<EffectResult> {
        if (i == 0) {
          EffectResult out;
          i++;
          const auto winner = GameEnded(battle.state);
          if (winner.has_value()) {
            std::vector<std::size_t> winners;
            for (const auto w : winner.value()) {
              winners.push_back(w == NACPlayer::NOUGHT ? 0U : 1U);
            }

            out.winners = winners;
          }
          return out;
        } else {
          return std::nullopt;
        }
      }
    }
  };
}

[[nodiscard]] std::optional<std::vector<NACPlayer>> GameEnded(const NACState &battle) {
  for (std::size_t i = 0; i < 3; i++) {
    // row is equal
    if ((battle.board[i][0].has_value()) && (battle.board[i][0] == battle.board[i][1]) && (battle.board[i][0] == battle.board[i][2])) {
      return std::vector{battle.board[i][0].value()};
    }

    // column is equal
    if ((battle.board[0][i].has_value()) && (battle.board[0][i] == battle.board[1][i]) && (battle.board[0][i] == battle.board[2][i])) {
      return std::vector{battle.board[0][i].value()};
    }
  }

  if ((battle.board[0][0].has_value()) && (battle.board[0][0] == battle.board[1][1]) && (battle.board[0][0] == battle.board[2][2])) {
    return std::vector{battle.board[0][0].value()};
  }

  if ((battle.board[2][0].has_value()) && (battle.board[2][0] == battle.board[1][1]) && (battle.board[2][0] == battle.board[0][2])) {
    return std::vector{battle.board[2][0].value()};
  }

  // check board still has empty spaces
  for (std::size_t x = 0; x < 3; x++) {
    for (std::size_t y = 0; y < 3; y++) {
      if (!battle.board[x][y].has_value()) {
        return std::nullopt;
      }
    }
  }

  return std::vector<NACPlayer>{};
}

} // namespace ngl::tbc::sample::nac
