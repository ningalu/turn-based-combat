#include <catch2/catch_test_macros.hpp>

#include <cstring>
#include <stdexcept>
#include <string>

#include "samples/NoughtsAndCrosses/lib/NACTypes.h"

TEST_CASE("Winner detection check is accurate", "[NACTypes]") {
  using namespace ngl::tbc::sample::nac; // NOLINT
  ngl::tbc::GridLayout<std::optional<NACPlayer>, 3> empty;

  ngl::tbc::GridLayout<std::optional<NACPlayer>, 3> horizontal_nought{
    std::array<std::optional<NACPlayer>, 3>{NACPlayer::NOUGHT, NACPlayer::NOUGHT, NACPlayer::NOUGHT}, // NOLINT
    std::array<std::optional<NACPlayer>, 3>{std::nullopt, std::nullopt, std::nullopt},                // NOLINT
    std::array<std::optional<NACPlayer>, 3>{std::nullopt, std::nullopt, std::nullopt}                 // NOLINT
  };
  ngl::tbc::GridLayout<std::optional<NACPlayer>, 3> vertical_nought{
    std::array<std::optional<NACPlayer>, 3>{NACPlayer::NOUGHT, std::nullopt, std::nullopt}, // NOLINT
    std::array<std::optional<NACPlayer>, 3>{NACPlayer::NOUGHT, std::nullopt, std::nullopt}, // NOLINT
    std::array<std::optional<NACPlayer>, 3>{NACPlayer::NOUGHT, std::nullopt, std::nullopt}  // NOLINT
  };

  ngl::tbc::GridLayout<std::optional<NACPlayer>, 3> diagonal_nought_1{
    std::array<std::optional<NACPlayer>, 3>{NACPlayer::NOUGHT, std::nullopt, std::nullopt}, // NOLINT
    std::array<std::optional<NACPlayer>, 3>{std::nullopt, NACPlayer::NOUGHT, std::nullopt}, // NOLINT
    std::array<std::optional<NACPlayer>, 3>{std::nullopt, std::nullopt, NACPlayer::NOUGHT}  // NOLINT
  };
  ngl::tbc::GridLayout<std::optional<NACPlayer>, 3> diagonal_nought_2{
    std::array<std::optional<NACPlayer>, 3>{std::nullopt, std::nullopt, NACPlayer::NOUGHT}, // NOLINT
    std::array<std::optional<NACPlayer>, 3>{std::nullopt, NACPlayer::NOUGHT, std::nullopt}, // NOLINT
    std::array<std::optional<NACPlayer>, 3>{NACPlayer::NOUGHT, std::nullopt, std::nullopt}  // NOLINT
  };
  ngl::tbc::GridLayout<std::optional<NACPlayer>, 3> diagonal_cross_1{
    std::array<std::optional<NACPlayer>, 3>{NACPlayer::CROSS, std::nullopt, std::nullopt}, // NOLINT
    std::array<std::optional<NACPlayer>, 3>{std::nullopt, NACPlayer::CROSS, std::nullopt}, // NOLINT
    std::array<std::optional<NACPlayer>, 3>{std::nullopt, std::nullopt, NACPlayer::CROSS}  // NOLINT
  };
  ngl::tbc::GridLayout<std::optional<NACPlayer>, 3> diagonal_cross_2{
    std::array<std::optional<NACPlayer>, 3>{std::nullopt, std::nullopt, NACPlayer::CROSS}, // NOLINT
    std::array<std::optional<NACPlayer>, 3>{std::nullopt, NACPlayer::CROSS, std::nullopt}, // NOLINT
    std::array<std::optional<NACPlayer>, 3>{NACPlayer::CROSS, std::nullopt, std::nullopt}  // NOLINT
  };

  ngl::tbc::GridLayout<std::optional<NACPlayer>, 3> draw{
    std::array<std::optional<NACPlayer>, 3>{NACPlayer::NOUGHT, NACPlayer::NOUGHT, NACPlayer::CROSS}, // NOLINT
    std::array<std::optional<NACPlayer>, 3>{NACPlayer::CROSS, NACPlayer::CROSS, NACPlayer::NOUGHT},  // NOLINT
    std::array<std::optional<NACPlayer>, 3>{NACPlayer::NOUGHT, NACPlayer::CROSS, NACPlayer::NOUGHT}  // NOLINT
  };

  for (const auto &[board, winner] : std::vector<std::pair<ngl::tbc::GridLayout<std::optional<NACPlayer>, 3>, std::optional<std::vector<NACPlayer>>>>{
         {empty, std::nullopt},
         {horizontal_nought, std::vector<NACPlayer>{NACPlayer::NOUGHT}},
         {vertical_nought, std::vector<NACPlayer>{NACPlayer::NOUGHT}},
         {diagonal_nought_1, std::vector<NACPlayer>{NACPlayer::NOUGHT}},
         {diagonal_nought_2, std::vector<NACPlayer>{NACPlayer::NOUGHT}},
         {diagonal_cross_1, std::vector<NACPlayer>{NACPlayer::CROSS}},
         {diagonal_cross_2, std::vector<NACPlayer>{NACPlayer::CROSS}},
         {draw, std::vector<NACPlayer>{}}
       }) {
    NACState state;
    state.board = board;
    CHECK(GameEnded(state) == winner);
  }

  CHECK(true);
}
