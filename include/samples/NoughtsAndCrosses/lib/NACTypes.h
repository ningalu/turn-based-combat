#include <string>

#include "tbc/BattleTypes.hpp"
#include "tbc/util/layout/GridLayout.hpp"

// Noughts and Crosses is an example of a game with perfectly sequential Turn order using
// only immediate commands

namespace ngl::tbc::sample::nac {

using NACCommand = ngl::tbc::GridLayoutIndex;
[[nodiscard]] std::optional<NACCommand> string_to_coord(const std::string &in);

enum class NACPlayer {
  NOUGHT,
  CROSS
};
[[nodiscard]] char str(NACPlayer p);

struct NACState {
  ngl::tbc::GridLayout<std::optional<NACPlayer>, 3U> board;
  [[nodiscard]] std::string str() const;
};

using NACTypes       = BattleTypes<NACState, Command<NACCommand>, CommandResult<bool>, Event<>>;
using Game           = NACTypes::TBattle;
using Command        = NACTypes::TCommand;
using CommandPayload = NACTypes::TCommandPayload;
using CommandResult  = NACTypes::TCommandResult;
using Scheduler      = NACTypes::TBattleScheduler;
using Schedule       = NACTypes::TSchedule;
using Action         = NACTypes::TAction;
using EffectResult   = NACTypes::TEffectResult;

[[nodiscard]] std::pair<std::optional<std::vector<CommandPayload>>, CommandResult> ValidateCommands(std::size_t player, const std::vector<CommandPayload> &commands, const Game &battle);

[[nodiscard]] Schedule GenerateSchedule(const Game &battle, const std::vector<Command> &buffered_commands, std::size_t turn);

[[nodiscard]] Action TranslateAction(const Command &command, const Game &battle);

[[nodiscard]] std::vector<Action> ActionEndHandler(ngl::tbc::DefaultEvents::PlannedActionEnd e, Game &battle);

[[nodiscard]] std::optional<std::vector<NACPlayer>> GameEnded(const NACState &battle);

} // namespace ngl::tbc::sample::nac