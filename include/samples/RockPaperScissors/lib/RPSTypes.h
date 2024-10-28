#include <optional>
#include <string>

#include "tbc/BattleTypes.hpp"

namespace ngl::tbc::sample::rps {
// TODO: worth allowing void/no battle state template parameter?
// only extremely trivial games have no state, to be fair
struct RPSState {};

enum class Play {
  ROCK,
  PAPER,
  SCISSORS
};

struct RPSCommand {
  Play play;
};

[[nodiscard]] std::optional<RPSCommand> from_str(const std::string &str);

using RPS          = BattleTypes<RPSState, Command<RPSCommand>, CommandResult<bool>, Event<>, SimultaneousActionStrategy::ENABLED>;
using Game         = RPS::TBattle;
using Cmd          = RPS::TCommand;
using CmdPayload   = RPS::TCommandPayload;
using CmdResult    = RPS::TCommandResult;
using Schedule     = RPS::TSchedule;
using Scheduler    = RPS::TBattleScheduler;
using Act          = RPS::TAction;
using Effect       = RPS::TEffect;
using EffectResult = RPS::TEffectResult;
using Player       = RPS::TPlayerComms;
using Actable      = RPS::TActionable;

// TODO: THIS SUCKS
[[nodiscard]] std::pair<std::optional<std::vector<CmdPayload>>, CmdResult> ValidateCommands(std::size_t player, const std::vector<CmdPayload> &commands, const Game &battle);
[[nodiscard]] Schedule GenerateSchedule(Game &battle, const std::vector<Cmd> &buffered_commands, std::size_t turn);
[[nodiscard]] Act TranslateAction(const std::vector<Cmd> &command, const Game &battle_arg);

} // namespace ngl::tbc::sample::rps