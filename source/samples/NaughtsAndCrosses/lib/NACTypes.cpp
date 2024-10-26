#include "samples/NaughtsAndCrosses/lib/NACTypes.h"

#include <stdexcept>

namespace ngl::tbc::sample::nac {

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

} // namespace ngl::tbc::sample::nac
