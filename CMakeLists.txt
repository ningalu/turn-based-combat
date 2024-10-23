cmake_minimum_required(VERSION 3.14)

include(cmake/prelude.cmake)

project(
    ngl-tbc-2
    VERSION 0.3.0
    DESCRIPTION "Short description"
    HOMEPAGE_URL "https://example.com/"
    LANGUAGES CXX
)

include(cmake/project-is-top-level.cmake)
include(cmake/variables.cmake)

# ---- Declare library ----

add_subdirectory(source/tbc)

# ---- Declare executable ----

add_executable(ngl-tbc-2_exe source/main.cpp)
add_executable(ngl-tbc-2::exe ALIAS ngl-tbc-2_exe)

set_property(TARGET ngl-tbc-2_exe PROPERTY OUTPUT_NAME ngl-tbc-2)

target_compile_features(ngl-tbc-2_exe PRIVATE cxx_std_20)

target_link_libraries(ngl-tbc-2_exe PRIVATE ngl-tbc_lib)

target_include_directories(
  ngl-tbc-2_exe 
  PUBLIC 
  "\$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/>"
)

# ---- Install rules ----

if(NOT CMAKE_SKIP_INSTALL_RULES)
  include(cmake/install-rules.cmake)
endif()

# ---- Developer mode ----

if(NOT ngl-tbc-2_DEVELOPER_MODE)
  return()
elseif(NOT PROJECT_IS_TOP_LEVEL)
  message(
      AUTHOR_WARNING
      "Developer mode is intended for developers of ngl-tbc-2"
  )
endif()

include(cmake/dev-mode.cmake)
