#include <catch2/catch_test_macros.hpp>

#include <stdexcept>
#include <string>

#include "tbc/EventHandler.hpp"

struct TestBattleState {};

using TestCommands       = ngl::tbc::Command<int, std::string>;
using TestCommandResults = ngl::tbc::CommandResult<bool>;
using TestEvents         = ngl::tbc::Event<int, double>;
using TestBattle         = ngl::tbc::Battle<TestBattleState, TestCommands, TestCommandResults, TestEvents, ngl::tbc::SimultaneousActionStrategy::DISABLED>;

using TestEffect = ngl::tbc::Effect<TestBattle, TestCommands, TestEvents>;
using TestAction = ngl::tbc::Action<TestBattle, TestCommands, TestEvents>;
TEST_CASE("Event Handler can be instantiated", "[EventHandler]") {

  ngl::tbc::EventHandler<TestBattle, TestCommands, TestEvents> eh;
  REQUIRE_FALSE(eh.HasHandler<int>());
  const auto action = TestAction{std::vector<TestEffect>{}};
  std::vector<int> handler_output;
  const auto handler = [&](int i, [[maybe_unused]] TestBattle &battle) { handler_output.push_back(i); return std::vector<TestAction>{action}; };
  REQUIRE_NOTHROW(eh.RegisterHandler<int>(handler));
  TestBattle b{{}, {}};
  auto action_out = eh.PostEvent<int>(1, b);

  CHECK(handler_output.size() == 1);
  CHECK(handler_output.at(0) == 1);

  eh.ResetHandler<int>();
}
