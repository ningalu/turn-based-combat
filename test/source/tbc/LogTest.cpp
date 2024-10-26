#include <catch2/catch_test_macros.hpp>

#include <stdexcept>
#include <string>

#include "tbc/Log.h"

TEST_CASE("Logs store messages", "[Log]") {
  const auto original_message = std::string{"message"};
  ngl::tbc::Log log{1, original_message};
  const auto retrieved_message = log.Retrieve(0);
}
