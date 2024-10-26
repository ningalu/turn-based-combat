#include <catch2/catch_test_macros.hpp>

#include <cstring>
#include <stdexcept>
#include <string>

#include "util/util.h"

#include "samples/NoughtsAndCrosses/lib/NACTypes.h"

ngl::tbc::sample::nac::NACTypes::TPlayerComms get_comms(std::vector<ngl::tbc::sample::nac::NACTypes::TCommandPayload> &payloads) {
  return {
    "CPU",
    [&]([[maybe_unused]] auto &&whatever) mutable -> std::vector<ngl::tbc::sample::nac::NACTypes::TCommandPayload> {
      assert(!payloads.empty());
      const auto payload = payloads.at(0);
      payloads.erase(payloads.begin());
      return std::vector{ngl::tbc::sample::nac::NACTypes::TCommandPayload{payload}};
    }
  };
}

std::vector<ngl::tbc::sample::nac::NACTypes::TCommandPayload> get_payloads(std::vector<ngl::tbc::sample::nac::NACCommand> p) {
  std::vector<ngl::tbc::sample::nac::NACTypes::TCommandPayload> out;
  for (const auto payload : p) {
    out.push_back(payload);
  }
  return out;
}

ngl::tbc::sample::nac::Game get_battle(ngl::tbc::sample::nac::NACState state, std::vector<ngl::tbc::sample::nac::NACCommand> p1, std::vector<ngl::tbc::sample::nac::NACCommand> p2) {
  std::vector<ngl::tbc::sample::nac::NACTypes::TCommandPayload> p1_payloads;
  for (const auto p : p1) {
    p1_payloads.push_back(p);
  }
  std::vector<ngl::tbc::sample::nac::NACTypes::TCommandPayload> p2_payloads;
  for (const auto p : p2) {
    p2_payloads.push_back(p);
  }

  auto out = ngl::tbc::sample::nac::Game{state, {get_comms(p1_payloads), get_comms(p2_payloads)}};
  out.SetCommandValidator(ngl::tbc::sample::nac::ValidateCommands);
  return out;
}

SCENARIO("Noughts and Crosses Integration Test", "[NACIntegration]") {
  using namespace ngl::tbc::sample::nac;

  Scheduler scheduler;
  scheduler.SetScheduleGenerator(GenerateSchedule);
  scheduler.SetActionTranslator(TranslateAction);
  scheduler.SetHandler<ngl::tbc::DefaultEvents::PlannedActionEnd>(ActionEndHandler);
  WHEN("Integration Test 1") {
    auto p1_payloads = get_payloads({{0, 0}, {0, 1}, {0, 2}});
    auto p2_payloads = get_payloads({{1, 0}, {1, 1}, {1, 2}});

    // have to do it this way to get around thread local mutable lambda copies not mutating properly
    // TODO: find a better way to set up command requests that "consume" values
    auto battle = Game{{}, {get_comms(p1_payloads), get_comms(p2_payloads)}};
    battle.SetCommandValidator(ValidateCommands);
    const auto winners = scheduler.RunBattle(battle);
    CHECK(winners.size() == 1);
    CHECK(winners.at(0) == 0);
  }
  CHECK(true);
}
