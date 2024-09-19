#include <iostream>
#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <variant>

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
#include "tbc/Target.h"
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

using MyBattle       = ngl::tbc::Battle<int, MyBattleState>;
using MyEvents       = ngl::tbc::Event<int, double>;
using MyEventHandler = ngl::tbc::EventHandler<MyBattle, MyEvents>;

using MyEffect        = ngl::tbc::Effect<MyBattle>;
using MyDefEffect     = ngl::tbc::DeferredEffect<MyBattle>;
using MyUserEffect    = ngl::tbc::UserEffect<MyBattle>;
using MyDefUserEffect = ngl::tbc::DeferredUserEffect<MyBattle>;

const MyEffect debug_effect{
  [](MyBattle &b, const std::vector<ngl::tbc::Target> &) { std::cout << "debug effect\n"; b.EndBattle({0}); return ngl::tbc::EffectResult::Success{}; }
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

    return ngl::tbc::EffectResult::Success{}; }
};

MyUserEffect GetEffect(int state) {
  return MyUserEffect{[=](ngl::tbc::Slot::Index u, MyBattle &b, const std::vector<ngl::tbc::Target> &) {
    switch (u.side) {
      case 0: b.p1_state = state; break;
      case 1: b.p2_state = state; break;
      default: throw std::logic_error{"invalid side"};
    }
    std::cout << "setting " << u.side << " to " << state << "\n";
    return ngl::tbc::EffectResult::Success{}; }};
}

using MyAction = ngl::tbc::Action<MyBattle>;
MyAction GetAction(ngl::tbc::Slot::Index user, const std::vector<ngl::tbc::Target> &targets, int state) {
  if (state > 2) {
    throw std::logic_error{"invalid state"};
  }

  MyDefUserEffect e{user, {GetEffect(state)}, targets};
  return MyAction{e};
}

MyEventHandler eh;

auto main() -> int {
  eh.RegisterHandler<int>([&](int a) {std::cout << "int: " << a << "\n"; return GetAction({0, 0}, {}, 0); });
  std::cout << "ningalu tbc\n";

  auto p1 = std::make_unique<ngl::tbc::PlayerComms<MyCommandPayload>>("Player 1", [&]() {
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
  });
  auto p2 = std::make_unique<ngl::tbc::PlayerComms<MyCommandPayload>>("Player 2", []() { return std::vector<MyCommandPayload>({MyCommandPayload{ScissorsCommand{}}}); });
  std::vector<std::unique_ptr<ngl::tbc::PlayerComms<MyCommandPayload>>> players;
  players.emplace_back(std::move(p1));
  players.emplace_back(std::move(p2));

  auto l = ngl::tbc::Layout{{{{0}}, {{1}}}};

  auto b  = MyBattle{0, l};
  auto bs = ngl::tbc::BattleScheduler<MyCommands, MyBattle, MyEvents>{std::move(players)};

  bs.SetCommandValidator([](std::size_t, const std::vector<MyCommandPayload> &) { return true; });
  bs.SetActionTranslator([](const std::vector<MyCommands> &commands) {
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
  });
  // const auto commands = bs.RequestCommands({0, 1});
  // auto actions        = bs.GetActions(commands);

  // for (const auto &r : commands) {
  //   static_assert(std::is_same_v<std::decay_t<decltype(r.payload)>, MyCommandPayload>);
  //   std::visit([](auto &&p) {
  //     using T = std::decay_t<decltype(p)>;
  //     if constexpr (std::is_same_v<T, RockCommand>) {
  //       std::cout << "Rock\n";
  //     } else if constexpr (std::is_same_v<T, PaperCommand>) {
  //       std::cout << "Paper\n";
  //     } else if constexpr (std::is_same_v<T, ScissorsCommand>) {
  //       std::cout << "Scissors\n";
  //     } else {
  //       throw std::logic_error{"invalid comand"};
  //     }
  //   },
  //              r.payload);
  //   std::cout << r.player << "\n";
  // }

  // ngl::tbc::Turn<MyBattle> turn{actions};
  bs.SetHandler<ngl::tbc::DefaultEvents::TurnsEnd>([=](ngl::tbc::DefaultEvents::TurnsEnd) { return MyAction{MyDefEffect{{resolve_rps_effect}, {}}}; });
  // bs.RunTurn(turn, b);
  auto winners = bs.RunBattle(b);
  std::cout << "Winner: Player " << winners.at(0) + 1 << "\n";

  return 0;
}
