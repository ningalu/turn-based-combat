#include <iostream>
#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>
#include <variant>

#include "util/util.h"

#include "tbc/Battle.hpp"
#include "tbc/Command.hpp"
#include "tbc/CommandPayloadSet.hpp"
#include "tbc/Effect.hpp"
#include "tbc/Event.hpp"
#include "tbc/EventHandler.hpp"
#include "tbc/PlayerComms.hpp"
#include "tbc/Scheduler.hpp"

#include "tbc/BattleTypes.hpp"

struct RockCommand {};
struct PaperCommand {};
struct ScissorsCommand {};

using MyCommands       = ngl::tbc::Command<RockCommand, PaperCommand, ScissorsCommand>;
using MyCommandPayload = MyCommands::Payload;

using MyCommandResult = ngl::tbc::CommandResult<bool>;

struct MyBattleState {
  int p1_state, p2_state;
  bool ended = false;
};

using MyEvents = ngl::tbc::Event<int, double>;

using MyBattleTypes = ngl::tbc::BattleTypes<MyBattleState, MyCommands, MyCommandResult, MyEvents>;

using MyBattle       = MyBattleTypes::TBattle;
using MyScheduler    = MyBattleTypes::TScheduler;
using MyEventHandler = MyBattleTypes::TEventHandler;

using MyEffect = MyBattleTypes::TEffect;

using MyResult = MyBattleTypes::TEffectResult;
using MyCmdSet = MyBattleTypes::TCommandPayloadTypeSet;
using MyComms  = MyBattleTypes::TPlayerComms;

using MySched      = MyBattleTypes::TSchedule;
using MyActionable = MyBattleTypes::TActionable;

MyEffect DebugEffect(int n) {
  return MyEffect{[=]([[maybe_unused]] auto &&...whatever) {std::cout << "effect resolution: " << n << "\n"; return MyResult{}; }};
}

MyEffect end_battle_effect() {
  return MyEffect{[](MyBattle &b) {
    std::cout << "debug effect\n";
    b.EndBattle({0});
    return MyResult{};
  }};
}

MyEffect resolve_rps_effect() {
  return MyEffect{[](MyBattle &b) {
    int win_table[3][3] = {{0, -1, 1}, {1, 0, -1}, {-1, 1, 0}}; // NOLINT TODO: find a way to turn off warnings for the executable project

    const auto outcome = win_table[b.state.p1_state][b.state.p2_state];
    if (outcome == 1) {
      b.EndBattle({0});
    } else if (outcome == -1) {
      b.EndBattle({1});
    }

    return MyResult{}; }};
}

MyEffect GetEffect(std::size_t side, int state) {
  return MyEffect{[=](MyBattle &b) {
    switch (side) {
      case 0: b.state.p1_state = state; break;
      case 1: b.state.p2_state = state; break;
      default: throw std::logic_error{"invalid side"};
    }
    std::cout << "setting " <<side << " to " << state << "\n";
    return MyResult{}; }};
}

MyEffect GetEffectWithIntEvent(std::size_t side, int state) {
  return MyEffect{[=](MyBattle &b) {
    switch (side) {
    case 0:
      b.state.p1_state = state;
      break;
    case 1:
      b.state.p2_state = state;
      break;
    default:
      throw std::logic_error{"invalid side"};
    }
    std::cout << "setting " << side << " to " << state << "\n";
    MyResult out;
    MyEvents e;
    e.payload = 2;
    out.queued_actions.emplace_back(MyActionable{e});
    return out;
  }};
}

using MyAction = MyBattleTypes::TAction;
MyAction GetAction(std::size_t user, int state) {
  if (state > 2) {
    throw std::logic_error{"invalid state"};
  }

  return MyAction{GetEffect(user, state)};
}

MyAction GetActionWithIntEffect(std::size_t user, int state) {
  if (state > 2) {
    throw std::logic_error{"invalid state"};
  }

  return MyAction{GetEffectWithIntEvent(user, state)};
}

std::function<std::vector<MyCommandPayload>(const MyCmdSet &, const MyBattle &)> GetComms(int n) {
  assert(n < 3);
  return [=]([[maybe_unused]] auto &&...whatever) -> std::vector<MyCommandPayload> {
    switch (n) {
    case 0:
      return std::vector<MyCommandPayload>({MyCommandPayload{RockCommand{}}});
    case 1:
      return std::vector<MyCommandPayload>({MyCommandPayload{PaperCommand{}}});
    case 2:
      return std::vector<MyCommandPayload>({MyCommandPayload{ScissorsCommand{}}});
    default:
      ngl::tbc::unreachable();
    }
  };
}

auto default_validator = []([[maybe_unused]] std::size_t unused1, const std::vector<MyCommandPayload> &payload, [[maybe_unused]] const MyBattle &unused2) -> std::pair<std::optional<std::vector<MyCommandPayload>>, MyCommandResult> {
  const std::optional<std::vector<MyCommandPayload>> out{payload};
  return std::pair{out, MyCommandResult{true}};
};

auto default_translator = [](const MyCommands &command, [[maybe_unused]] const MyBattle &unused) {
  int move = std::visit([](auto &&p) {
    using T = std::decay_t<decltype(p)>;
    if constexpr (std::is_same_v<T, RockCommand>) {
      return 0;
    } else if constexpr (std::is_same_v<T, PaperCommand>) {
      return 1;
    } else if constexpr (std::is_same_v<T, ScissorsCommand>) {
      return 2;
    } else {
      throw std::logic_error{"invalid move"};
    }
  },
                        command.payload);

  return GetAction(command.player, move);
};

auto int_event_translator = [](const MyCommands &command, [[maybe_unused]] const MyBattle &unused) {
  int move = std::visit([](auto &&p) {
    using T = std::decay_t<decltype(p)>;
    if constexpr (std::is_same_v<T, RockCommand>) {
      return 0;
    } else if constexpr (std::is_same_v<T, PaperCommand>) {
      return 1;
    } else if constexpr (std::is_same_v<T, ScissorsCommand>) {
      return 2;
    } else {
      throw std::logic_error{"invalid move"};
    }
  },
                        command.payload);

  return GetActionWithIntEffect(command.player, move);
};

std::vector<MyAction> default_turnend([[maybe_unused]] ngl::tbc::DefaultEvents::ScheduleEnd unused1, [[maybe_unused]] const MyBattle &unused2) {
  return {MyAction{std::vector<MyEffect>{MyEffect{resolve_rps_effect()}}}};
}

std::vector<MyAction> default_intevent(int n, [[maybe_unused]] const MyBattle &unused) {
  std::cout << "int event received: " << n << "\n";
  return {MyAction{std::vector<MyEffect>{MyEffect{{DebugEffect(n)}}}}};
}

void test_battle_end();
void test_user_event();

MySched schedule_generator(MyBattle &battle, [[maybe_unused]] const std::vector<MyCommands> &commands, [[maybe_unused]] std::size_t turn) {
  // std::vector<MyBattleTypes::TCommandRequest> requests;
  // requests.push_back(MyBattleTypes::TCommandRequest{0, MyBattleTypes::TCommandPayloadTypeSet{true}});
  // requests.push_back(MyBattleTypes::TCommandRequest{1, MyBattleTypes::TCommandPayloadTypeSet{true}});
  // std::vector<MyCommands> out = battle.RequestCommands(requests);
  std::vector<MyCommands> out = battle.RequestCommands(std::vector<std::size_t>{{0, 1}});
  return MySched{out};
}

auto main() -> int {
  try {

    using mytypeset = ngl::tmp::typeset<int, double, char>;
    auto types1     = mytypeset{};
    assert(!types1.contains<int>());
    types1.insert<int>();
    assert(types1.contains<int>());
    types1.erase<int>();
    assert(!types1.contains<int>());

    std::cout << "Test battle end\n";
    test_battle_end();
    std::cout << "\n\nTest user event\n";
    test_user_event();

    auto input_comms = [&]([[maybe_unused]] auto &&...whatever) {
      std::set<std::string> valid{"R", "P", "S"};
      std::cout << "Play R | P | S\n";
      std::string in;
      while (!valid.contains(in)) {
        std::cin >> in;
      }
      if (in == "R") {
        return std::vector<MyCommandPayload>({MyCommandPayload{RockCommand{}}});
      }
      if (in == "P") {
        return std::vector<MyCommandPayload>({MyCommandPayload{PaperCommand{}}});
      }
      if (in == "S") {
        return std::vector<MyCommandPayload>({MyCommandPayload{ScissorsCommand{}}});
      }
      throw std::logic_error{"whatever"};
    };

    auto p1 = MyComms{"Player 1", input_comms};
    auto p2 = MyComms{"Player 2", GetComms(2)};

    std::vector<MyComms> players({p1, p2});

    auto b = MyBattle{players};
    b.SetCommandValidator(default_validator);

    MyScheduler bs;
    bs.SetActionTranslator(default_translator);
    bs.SetBattleEndedChecker([]([[maybe_unused]] auto &&unused) { return std::nullopt; });
    bs.SetHandler<ngl::tbc::DefaultEvents::ScheduleEnd>(default_turnend);
    bs.SetScheduleGenerator(schedule_generator);

    auto winners = bs.RunBattle(b);
    std::cout << "Winner: Player " << winners.at(0) + 1 << "\n";

    return 0;
  } catch (const std::exception &e) {
    std::cout << "exception thrown: " << e.what() << "\n";
    return 1;
  }
}

void test_battle_end() {

  auto p1 = MyComms{"Player 1", GetComms(1)};
  auto p2 = MyComms{"Player 2", GetComms(2)};

  std::vector<MyComms> players({p1, p2});

  auto b = MyBattle{players};
  b.SetCommandValidator(default_validator);

  MyScheduler bs;
  bs.SetActionTranslator(default_translator);
  bs.SetBattleEndedChecker([]([[maybe_unused]] auto &&unused) { return std::nullopt; });
  bs.SetHandler<ngl::tbc::DefaultEvents::ScheduleEnd>(default_turnend);
  bs.SetScheduleGenerator(schedule_generator);

  auto winners = bs.RunBattle(b);

  assert(winners.size() == 1);
  assert(winners.at(0) == 1);
}

void test_user_event() {
  auto p1 = MyComms{"Player 1", GetComms(1)};
  auto p2 = MyComms{"Player 2", GetComms(2)};

  std::vector<MyComms> players({p1, p2});

  auto b = MyBattle{players};
  b.SetCommandValidator(default_validator);

  MyScheduler bs;
  bs.SetActionTranslator(int_event_translator);
  bs.SetBattleEndedChecker([]([[maybe_unused]] auto &&unused) { return std::nullopt; });
  bs.SetHandler<ngl::tbc::DefaultEvents::ScheduleEnd>(default_turnend);
  bs.SetHandler<int>(default_intevent);
  bs.SetScheduleGenerator(schedule_generator);

  auto winners = bs.RunBattle(b);

  assert(winners.size() == 1);
  assert(winners.at(0) == 1);
}