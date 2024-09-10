#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <variant>

#include "tbc/Battle.hpp"
#include "tbc/BattleScheduler.hpp"
#include "tbc/Command.hpp"
#include "tbc/Effect.hpp"
#include "tbc/Layout.h"
#include "tbc/PlayerComms.hpp"
#include "tbc/Slot.h"
#include "tbc/Target.h"
#include "tbc/UserEffect.hpp"

struct RockCommand {};
struct PaperCommand {};
struct ScissorsCommand {};
using MyCommands       = ngl::tbc::Command<RockCommand, PaperCommand, ScissorsCommand>;
using MyCommandPayload = MyCommands::Payload;

struct MyBattleState {
  int p1_state, p2_state;
  bool ended = false;
};
using MyBattle = ngl::tbc::Battle<int, MyBattleState>;

using MyUserEffect = ngl::tbc::UserEffect<MyBattle>;

MyUserEffect rock_effect{
  [](ngl::tbc::Slot::Index u, MyBattle &b, const std::vector<ngl::tbc::Target> &) { 
    switch (u.side) {
      case 0: b.p1_state = 0; break;
      case 1: b.p2_state = 0; break;
      default: throw std::logic_error{"invalid side"};
    }
    return ngl::tbc::EffectResult::Success{}; }
};

MyUserEffect paper_effect{
  [](ngl::tbc::Slot::Index u, MyBattle &b, const std::vector<ngl::tbc::Target> &) { 
    switch (u.side) {
      case 0: b.p1_state = 1; break;
      case 1: b.p2_state = 1; break;
      default: throw std::logic_error{"invalid side"};
    }
    return ngl::tbc::EffectResult::Success{}; }
};

MyUserEffect scissor_effect{
  [](ngl::tbc::Slot::Index u, MyBattle &b, const std::vector<ngl::tbc::Target> &) { 
    switch (u.side) {
      case 0: b.p1_state = 2; break;
      case 1: b.p2_state = 2; break;
      default: throw std::logic_error{"invalid side"};
    }
    return ngl::tbc::EffectResult::Success{}; }
};

using MyAction = ngl::tbc::Action<MyUserEffect>;
MyAction GetAction(ngl::tbc::Slot::Index user, const std::vector<ngl::tbc::Target> &targets, int state) {
  std::vector<MyUserEffect> out;
  switch (state) {
  case 0:
    out.push_back(rock_effect);
    break;
  case 1:
    out.push_back(paper_effect);
    break;
  case 2:
    out.push_back(scissor_effect);
    break;
  default:
    throw std::logic_error{"invalid state"};
  }
  return MyAction{user, targets, [=]() mutable {
                    std::cout << out.size() << "\n";
                    std::optional<MyUserEffect> e;
                    if (out.size() > 0) {
                      e = out.at(0);
                      out.erase(out.begin());
                    }
                    return e;
                  }};
}

auto main() -> int {
  std::cout << "ningalu tbc\n";

  auto p1 = std::make_unique<ngl::tbc::PlayerComms<MyCommandPayload>>("Player 1", []() { return std::vector<MyCommandPayload>({MyCommandPayload{PaperCommand{}}}); });
  auto p2 = std::make_unique<ngl::tbc::PlayerComms<MyCommandPayload>>("Player 2", []() { return std::vector<MyCommandPayload>({MyCommandPayload{RockCommand{}}}); });
  std::vector<std::unique_ptr<ngl::tbc::PlayerComms<MyCommandPayload>>> players;
  players.emplace_back(std::move(p1));
  players.emplace_back(std::move(p2));

  auto l = ngl::tbc::Layout{{{{0}}, {{1}}}};

  auto b  = MyBattle{0, l};
  auto bs = ngl::tbc::BattleScheduler<MyCommands, MyBattle>{std::move(players)};

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

  for (const auto &r : bs.RequestCommands({0, 1})) {
    static_assert(std::is_same_v<std::decay_t<decltype(r.payload)>, MyCommandPayload>);
    std::visit([](auto &&p) {
      using T = std::decay_t<decltype(p)>;
      if constexpr (std::is_same_v<T, RockCommand>) {
        std::cout << "Rock\n";
      } else if constexpr (std::is_same_v<T, PaperCommand>) {
        std::cout << "Paper\n";
      } else if constexpr (std::is_same_v<T, ScissorsCommand>) {
        std::cout << "Scissors\n";
      } else {
        throw std::logic_error{"invalid comand"};
      }
    },
               r.payload);
    std::cout << r.player << "\n";
  }
  std::vector<std::optional<ngl::tbc::UserEffect<MyBattle>>> action_queue;
  action_queue.push_back(ngl::tbc::UserEffect<MyBattle>{[](ngl::tbc::Slot::Index, MyBattle &, const std::vector<ngl::tbc::Target> &) { return ngl::tbc::EffectResult::Success{}; }});
  std::size_t i = 0;
  ngl::tbc::Action<ngl::tbc::UserEffect<ngl::tbc::Battle<int, MyBattleState>>> a{ngl::tbc::Slot::Index{0, 0}, {}, [&i, &action_queue]() {
                                                                                   std::optional<ngl::tbc::UserEffect<MyBattle>> out = std::nullopt;
                                                                                   if (i < action_queue.size()) {
                                                                                     out = action_queue.at(i);
                                                                                   }
                                                                                   i++;
                                                                                   return out;
                                                                                 }};

  bs.ExecuteAction(a, b);

  bs.ExecuteAction(GetAction({0, 0}, {}, 1), b);

  return 0;
}
