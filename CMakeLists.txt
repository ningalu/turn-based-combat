cmake_minimum_required(VERSION 3.14)

include(cmake/prelude.cmake)

project(
    ningalu-turn-based-combat-2
    VERSION 0.0.0
    DESCRIPTION "Short description"
    HOMEPAGE_URL "https://example.com/"
    LANGUAGES CXX
)

include(cmake/project-is-top-level.cmake)
include(cmake/variables.cmake)

# ---- Declare library ----

add_library(
    ningalu-turn-based-combat-2_lib OBJECT
    source/lib.cpp
)

target_include_directories(
    ningalu-turn-based-combat-2_lib ${warning_guard}
    PUBLIC
    "\$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/source>"
)

target_compile_features(ningalu-turn-based-combat-2_lib PUBLIC cxx_std_20)

find_package(fmt REQUIRED)
target_link_libraries(ningalu-turn-based-combat-2_lib PRIVATE fmt::fmt)

# ---- Declare executable ----

add_executable(ningalu-turn-based-combat-2_exe source/main.cpp)
add_executable(ningalu-turn-based-combat-2::exe ALIAS ningalu-turn-based-combat-2_exe)

set_property(TARGET ningalu-turn-based-combat-2_exe PROPERTY OUTPUT_NAME ningalu-turn-based-combat-2)

target_compile_features(ningalu-turn-based-combat-2_exe PRIVATE cxx_std_20)

target_link_libraries(ningalu-turn-based-combat-2_exe PRIVATE ningalu-turn-based-combat-2_lib)

# ---- Install rules ----

if(NOT CMAKE_SKIP_INSTALL_RULES)
  include(cmake/install-rules.cmake)
endif()

# ---- Developer mode ----

if(NOT ningalu-turn-based-combat-2_DEVELOPER_MODE)
  return()
elseif(NOT PROJECT_IS_TOP_LEVEL)
  message(
      AUTHOR_WARNING
      "Developer mode is intended for developers of ningalu-turn-based-combat-2"
  )
endif()

include(cmake/dev-mode.cmake)
