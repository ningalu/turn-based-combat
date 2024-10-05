#include <iostream>
#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>
#include <variant>

#include "util.h"

#include "tbc/Battle.hpp"
#include "tbc/BattleScheduler.hpp"
#include "tbc/Command.hpp"
#include "tbc/DeferredEffect.hpp"
#include "tbc/DeferredUserEffect.hpp"
#include "tbc/Effect.hpp"
#include "tbc/Event.hpp"
#include "tbc/EventHandler.hpp"
#include "tbc/Layout.h"
#include "tbc/PlayerComms.hpp"
#include "tbc/Slot.h"
#include "tbc/Target.hpp"
#include "tbc/Turn.hpp"
#include "tbc/UserEffect.hpp"

#include "tbc/BattleTypes.hpp"

struct RockCommand {};
struct PaperCommand {};
struct ScissorsCommand {};

using MyCommands       = ngl::tbc::Command<RockCommand, PaperCommand, ScissorsCommand>;
using MyCommandPayload = MyCommands::Payload;

struct MyBattleState {
  int p1_state, p2_state;
  bool ended = false;
};

using MyEvents = ngl::tbc::Event<int, double>;

using MyBattleTypes = ngl::tbc::BattleTypes<int, MyBattleState, MyCommands, MyEvents>;

using MyBattle       = MyBattleTypes::Battle;
using MyScheduler    = MyBattleTypes::BattleScheduler;
using MyEventHandler = MyBattleTypes::EventHandler;

using MyEffect        = MyBattleTypes::Effect;
using MyDefEffect     = MyBattleTypes::DeferredEffect;
using MyUserEffect    = MyBattleTypes::UserEffect;
using MyDefUserEffect = MyBattleTypes::DeferredUserEffect;

using MyResult = MyBattleTypes::EffectResult;

using MyComms = MyBattleTypes::PlayerComms;

MyEffect DebugEffect(int n) {
  return {[=](auto &&...) {std::cout << "effect resolution: " << n << "\n"; return MyResult{}; }};
}

const MyEffect end_battle_effect{
  [](MyBattle &b, const std::vector<ngl::tbc::Target> &) { std::cout << "debug effect\n"; b.EndBattle({0}); return MyResult{}; }
};

const MyEffect resolve_rps_effect{
  [](MyBattle &b, const std::vector<ngl::tbc::Target> &) {
    int win_table[3][3] = {{0, -1, 1}, {1, 0, -1}, {-1, 1, 0}};

    const auto outcome = win_table[b.p1_state][b.p2_state];
    if (outcome == 1) {
      b.EndBattle({0});
    } else if (outcome == -1) {
      b.EndBattle({1});
    }

    return MyResult{}; }
};

MyUserEffect GetEffect(int state) {
  return MyUserEffect{[=](ngl::tbc::Slot::Index u, MyBattle &b, const std::vector<ngl::tbc::Target> &) {
    switch (u.side) {
      case 0: b.p1_state = state; break;
      case 1: b.p2_state = state; break;
      default: throw std::logic_error{"invalid side"};
    }
    std::cout << "setting " << u.side << " to " << state << "\n";
    return MyResult{}; }};
}

MyUserEffect GetEffectWithIntEvent(int state) {
  return MyUserEffect{[=](ngl::tbc::Slot::Index u, MyBattle &b, const std::vector<ngl::tbc::Target> &) {
    switch (u.side) {
      case 0: b.p1_state = state; break;
      case 1: b.p2_state = state; break;
      default: throw std::logic_error{"invalid side"};
    }
    std::cout << "setting " << u.side << " to " << state << "\n";
    MyResult out;
    MyEvents e;
    e.payload = 2;
    std::get<3>(out).events.push_back(e);
    return out; }};
}

using MyAction = MyBattleTypes::Action;
MyAction GetAction(ngl::tbc::Slot::Index user, const std::vector<ngl::tbc::Target> &targets, int state) {
  if (state > 2) {
    throw std::logic_error{"invalid state"};
  }

  MyDefUserEffect e{user, GetEffect(state), targets};
  return MyAction{std::vector<MyDefUserEffect>{e}};
}

MyAction GetActionWithIntEffect(ngl::tbc::Slot::Index user, const std::vector<ngl::tbc::Target> &targets, int state) {
  if (state > 2) {
    throw std::logic_error{"invalid state"};
  }

  MyDefUserEffect e{user, {GetEffectWithIntEvent(state)}, targets};
  return MyAction{std::vector<MyDefUserEffect>{e}};
}

std::function<std::vector<MyCommandPayload>()> GetComms(int n) {
  assert(n < 3);
  return [=]() {
    switch (n) {
    case 0:
      return std::vector<MyCommandPayload>({MyCommandPayload{RockCommand{}}});
    case 1:
      return std::vector<MyCommandPayload>({MyCommandPayload{PaperCommand{}}});
    case 2:
      return std::vector<MyCommandPayload>({MyCommandPayload{ScissorsCommand{}}});
    default:
      ngl::unreachable();
    }
  };
};

auto default_validator  = [](std::size_t, const std::vector<MyCommandPayload> &payload, const std::vector<MyCommands> &) { return payload; };
auto default_translator = [](const std::vector<MyCommands> &commands) {
  auto out = std::vector<MyAction>{};
  for (const auto &c : commands) {
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
                          c.payload);

    out.push_back(GetAction({c.player, 0}, {}, move));
  }
  return out;
};

auto int_event_translator = [](const std::vector<MyCommands> &commands) {
  auto out = std::vector<MyAction>{};
  for (const auto &c : commands) {
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
                          c.payload);

    out.push_back(GetActionWithIntEffect({c.player, 0}, {}, move));
  }
  return out;
};

MyAction default_turnend(ngl::tbc::DefaultEvents::TurnsEnd) {
  return MyAction{std::vector<MyDefEffect>{MyDefEffect{resolve_rps_effect, {}}}};
}

MyAction default_intevent(int n) {
  std::cout << "int event received: " << n << "\n";
  return MyAction{std::vector<MyDefEffect>{MyDefEffect{{DebugEffect(n)}, {}}}};
}

auto default_layout = ngl::tbc::Layout{{{{0}}, {{1}}}};

void test_battle_end();
void test_user_event();

auto main() -> int {
  std::cout << "Test battle end\n";
  test_battle_end();
  std::cout << "\n\nTest user event\n";
  test_user_event();

  auto input_comms = [&]() {
    std::set<std::string> valid{"R", "P", "S"};
    std::cout << "Play R | P | S\n";
    std::string in;
    while (!valid.contains(in)) {
      std::cin >> in;
    }
    if (in == "R") {
      return std::vector<MyCommandPayload>({MyCommandPayload{RockCommand{}}});
    } else if (in == "P") {
      return std::vector<MyCommandPayload>({MyCommandPayload{PaperCommand{}}});
    } else {
      return std::vector<MyCommandPayload>({MyCommandPayload{ScissorsCommand{}}});
    }
  };

  auto p1 = std::make_unique<MyComms>("Player 1", input_comms);
  auto p2 = std::make_unique<MyComms>("Player 2", GetComms(2));

  std::vector<std::unique_ptr<MyComms>> players;
  players.emplace_back(std::move(p1));
  players.emplace_back(std::move(p2));

  auto b  = MyBattle{0, default_layout};
  auto bs = MyScheduler{std::move(players)};

  bs.SetCommandValidator(default_validator);
  bs.SetActionTranslator(default_translator);

  bs.SetHandler<ngl::tbc::DefaultEvents::TurnsEnd>(default_turnend);

  auto winners = bs.RunBattle(b);
  std::cout << "Winner: Player " << winners.at(0) + 1 << "\n";

  return 0;
}

void test_battle_end() {

  auto p1 = std::make_unique<MyComms>("Player 1", GetComms(1));
  auto p2 = std::make_unique<MyComms>("Player 2", GetComms(2));

  std::vector<std::unique_ptr<MyComms>> players;
  players.emplace_back(std::move(p1));
  players.emplace_back(std::move(p2));

  auto b  = MyBattle{0, default_layout};
  auto bs = MyScheduler{std::move(players)};
  bs.SetCommandValidator(default_validator);
  bs.SetActionTranslator(default_translator);
  bs.SetHandler<ngl::tbc::DefaultEvents::TurnsEnd>(default_turnend);

  auto winners = bs.RunBattle(b);

  assert(winners.size() == 1);
  assert(winners.at(0) == 1);
}

void test_user_event() {
  auto p1 = std::make_unique<MyComms>("Player 1", GetComms(1));
  auto p2 = std::make_unique<MyComms>("Player 2", GetComms(2));

  std::vector<std::unique_ptr<MyComms>> players;
  players.emplace_back(std::move(p1));
  players.emplace_back(std::move(p2));

  auto b  = MyBattle{0, default_layout};
  auto bs = MyScheduler{std::move(players)};
  bs.SetCommandValidator(default_validator);
  bs.SetActionTranslator(int_event_translator);
  bs.SetHandler<ngl::tbc::DefaultEvents::TurnsEnd>(default_turnend);
  bs.SetHandler<int>(default_intevent);

  auto winners = bs.RunBattle(b);

  assert(winners.size() == 1);
  assert(winners.at(0) == 1);
}