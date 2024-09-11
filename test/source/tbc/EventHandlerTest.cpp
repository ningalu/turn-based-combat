#include <catch2/catch_test_macros.hpp>

#include <stdexcept>

#include "tbc/EventHandler.hpp"

struct TestBattleState {};

using TestBattle = ngl::tbc::Battle<int, TestBattleState>;
using TestEvents = ngl::tbc::Event<TestBattle, int, double>;

TEST_CASE("Event Handler can be instantiated", "[EventHandler]") {

  ngl::tbc::EventHandler<TestEvents> eh;
  REQUIRE_FALSE(eh.HasHandler<int>());
  const auto action  = ngl::tbc::Action<TestBattle>{ngl::tbc::DeferredEffect<TestBattle>{{}, {}}};
  const auto handler = [=](int) { return action; };
  REQUIRE_NOTHROW(eh.RegisterHandler<int>(handler));
  auto action_out = eh.PostEvent<int>(1);

  eh.ResetHandler<int>();
}
