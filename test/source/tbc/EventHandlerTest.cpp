#include <catch2/catch_test_macros.hpp>

#include <stdexcept>

#include "tbc/EventHandler.hpp"

struct TestBattleState {};

using TestBattle = ngl::tbc::Battle<int, TestBattleState>;
using TestEvents = ngl::tbc::Event<int, double>;

TEST_CASE("Event Handler can be instantiated", "[EventHandler]") {

  ngl::tbc::EventHandler<TestBattle, TestEvents> eh;
  REQUIRE_FALSE(eh.HasHandler<int>());
  const auto action = ngl::tbc::Action<TestBattle>{ngl::tbc::DeferredEffect<TestBattle>{{}, {}}};
  std::vector<int> handler_output;
  const auto handler = [&](int i) { handler_output.push_back(i); return action; };
  REQUIRE_NOTHROW(eh.RegisterHandler<int>(handler));
  auto action_out = eh.PostEvent<int>(1);

  CHECK(handler_output.size() == 1);
  CHECK(handler_output.at(0) == 1);

  eh.ResetHandler<int>();
}
