#include <catch2/catch_test_macros.hpp>

#include <cstring>
#include <stdexcept>
#include <string>

#include "samples/NoughtsAndCrosses/lib/NACTypes.h"

TEST_CASE("Winner detection check is accurate", "[NACTypes]") {
  using namespace ngl::tbc::sample::nac;
  std::optional<NACPlayer> empty[3][3] = {std::nullopt};

  std::optional<NACPlayer> horizontal_nought[3][3] = {
    {NACPlayer::NOUGHT, NACPlayer::NOUGHT, NACPlayer::NOUGHT},
    {std::nullopt, std::nullopt, std::nullopt},
    {std::nullopt, std::nullopt, std::nullopt}
  };
  std::optional<NACPlayer> vertical_nought[3][3] = {
    {NACPlayer::NOUGHT, std::nullopt, std::nullopt},
    {NACPlayer::NOUGHT, std::nullopt, std::nullopt},
    {NACPlayer::NOUGHT, std::nullopt, std::nullopt}
  };

  std::optional<NACPlayer> diagonal_nought_1[3][3] = {
    {NACPlayer::NOUGHT, std::nullopt, std::nullopt},
    {std::nullopt, NACPlayer::NOUGHT, std::nullopt},
    {std::nullopt, std::nullopt, NACPlayer::NOUGHT}
  };
  std::optional<NACPlayer> diagonal_nought_2[3][3] = {
    {std::nullopt, std::nullopt, NACPlayer::NOUGHT},
    {std::nullopt, NACPlayer::NOUGHT, std::nullopt},
    {NACPlayer::NOUGHT, std::nullopt, std::nullopt}
  };
  std::optional<NACPlayer> diagonal_cross_1[3][3] = {
    {NACPlayer::CROSS, std::nullopt, std::nullopt},
    {std::nullopt, NACPlayer::CROSS, std::nullopt},
    {std::nullopt, std::nullopt, NACPlayer::CROSS}
  };
  std::optional<NACPlayer> diagonal_cross_2[3][3] = {
    {std::nullopt, std::nullopt, NACPlayer::CROSS},
    {std::nullopt, NACPlayer::CROSS, std::nullopt},
    {NACPlayer::CROSS, std::nullopt, std::nullopt}
  };

  std::optional<NACPlayer> draw[3][3] = {
    {NACPlayer::NOUGHT, NACPlayer::NOUGHT, NACPlayer::CROSS},
    {NACPlayer::CROSS, NACPlayer::CROSS, NACPlayer::NOUGHT},
    {NACPlayer::NOUGHT, NACPlayer::CROSS, NACPlayer::NOUGHT}
  };

  for (const auto [board, winner] : std::vector<std::pair<decltype(&empty), std::optional<std::vector<NACPlayer>>>>{
         {&empty, std::nullopt},
         {&horizontal_nought, std::vector<NACPlayer>{NACPlayer::NOUGHT}},
         {&vertical_nought, std::vector<NACPlayer>{NACPlayer::NOUGHT}},
         {&diagonal_nought_1, std::vector<NACPlayer>{NACPlayer::NOUGHT}},
         {&diagonal_nought_2, std::vector<NACPlayer>{NACPlayer::NOUGHT}},
         {&diagonal_cross_1, std::vector<NACPlayer>{NACPlayer::CROSS}},
         {&diagonal_cross_2, std::vector<NACPlayer>{NACPlayer::CROSS}},
         {&draw, std::vector<NACPlayer>{}}
       }) {
    NACState state;
    for (std::size_t i = 0; i < 3; i++) {
      for (std::size_t j = 0; j < 3; j++) {
        state.board[i][j] = (*board)[i][j]; // this is stupid
      }
    }
    CHECK(GameEnded(state) == winner);
  }

  CHECK(true);
}
