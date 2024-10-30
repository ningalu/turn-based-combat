#include <catch2/catch_test_macros.hpp>

#include <cstring>
#include <functional>
#include <stdexcept>
#include <string>
#include <vector>

#include "util/util.h"

#include "samples/RockPaperScissors/lib/RPSTypes.h"

ngl::tbc::sample::rps::RPS::TPlayerComms get_comms(std::vector<ngl::tbc::sample::rps::RPS::TCommandPayload> &payloads) {
  return {
    "CPU",
    [&]([[maybe_unused]] auto &&...whatever) mutable -> std::vector<ngl::tbc::sample::rps::RPS::TCommandPayload> {
      assert(!payloads.empty());
      const auto payload = payloads.at(0);
      payloads.erase(payloads.begin());
      return std::vector{ngl::tbc::sample::rps::RPS::TCommandPayload{payload}};
    }
  };
}

std::vector<ngl::tbc::sample::rps::RPS::TCommandPayload> get_payloads(std::vector<ngl::tbc::sample::rps::RPSCommand> p) {
  std::vector<ngl::tbc::sample::rps::RPS::TCommandPayload> out;
  for (const auto payload : p) {
    out.push_back(payload);
  }
  return out;
}

SCENARIO("Rock Paper Scissors Integration Test", "[RPSIntegration]") {
  using namespace ngl::tbc::sample::rps; // NOLINT

  Scheduler s;
  s.SetScheduleGenerator(GenerateSchedule);
  s.SetActionTranslator(TranslateAction);

  GIVEN("Paper beats Rock") {
    auto p1 = get_payloads({RPSCommand{Play::ROCK}});
    auto p2 = get_payloads({RPSCommand{Play::PAPER}});
    Game g{{get_comms(p1), get_comms(p2)}};
    g.SetCommandValidator(ValidateCommands);

    const auto winner = s.RunBattle(g);
    CHECK(p1.empty());
    CHECK(p2.empty());
    CHECK(winner.size() == 1);
    CHECK(winner.front() == 1);
  }

  GIVEN("Scissors beats Paper") {
    auto p1 = get_payloads({RPSCommand{Play::SCISSORS}});
    auto p2 = get_payloads({RPSCommand{Play::PAPER}});
    Game g{{get_comms(p1), get_comms(p2)}};
    g.SetCommandValidator(ValidateCommands);

    const auto winner = s.RunBattle(g);
    CHECK(p1.empty());
    CHECK(p2.empty());
    CHECK(winner.size() == 1);
    CHECK(winner.front() == 0);
  }

  GIVEN("Rock beats Scissors") {
    auto p1 = get_payloads({RPSCommand{Play::SCISSORS}});
    auto p2 = get_payloads({RPSCommand{Play::ROCK}});
    Game g{{get_comms(p1), get_comms(p2)}};
    g.SetCommandValidator(ValidateCommands);

    const auto winner = s.RunBattle(g);
    CHECK(p1.empty());
    CHECK(p2.empty());
    CHECK(winner.size() == 1);
    CHECK(winner.front() == 1);
  }

  GIVEN("Draws continue the game") {
    auto p1 = get_payloads({RPSCommand{Play::SCISSORS}, RPSCommand{Play::SCISSORS}});
    auto p2 = get_payloads({RPSCommand{Play::SCISSORS}, RPSCommand{Play::ROCK}});
    Game g{{get_comms(p1), get_comms(p2)}};
    g.SetCommandValidator(ValidateCommands);

    const auto winner = s.RunBattle(g);
    CHECK(p1.empty());
    CHECK(p2.empty());
    CHECK(winner.size() == 1);
    CHECK(winner.front() == 1);
  }
}
