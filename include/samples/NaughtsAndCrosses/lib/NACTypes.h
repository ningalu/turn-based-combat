#include <string>

#include "tbc/BattleTypes.hpp"

namespace ngl::tbc::sample::nac {
struct NACCommand {
  uint8_t x, y;
};

enum class NACPlayer {
  NOUGHT,
  CROSS
};
[[nodiscard]] char str(NACPlayer p);

struct NACState {
  std::optional<NACPlayer> board[3][3];
  [[nodiscard]] std::string str() const;
};

using NACTypes       = BattleTypes<int, NACState, Command<NACCommand>, CommandResult<bool>, Event<>>;
using Game           = NACTypes::TBattle;
using Command        = NACTypes::TCommand;
using CommandPayload = NACTypes::TCommandPayload;
using CommandResult  = NACTypes::TCommandResult;
using Scheduler      = NACTypes::TBattleScheduler;
using Schedule       = NACTypes::TSchedule;
using Action         = NACTypes::TAction;
using EffectResult   = NACTypes::TEffectResult;

[[nodiscard]] std::pair<std::optional<std::vector<CommandPayload>>, CommandResult> ValidateCommands(std::size_t player, const std::vector<CommandPayload> &commands, const Game &battle);

[[nodiscard]] Schedule GenerateSchedule(Game &battle, const std::vector<Command> &buffered_commands, std::size_t turn);

[[nodiscard]] Action TranslateAction(const Command &command, const Game &battle);

} // namespace ngl::tbc::sample::nac