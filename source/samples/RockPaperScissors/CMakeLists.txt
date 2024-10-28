add_library(
  ngl-tbc-samples-rps_lib OBJECT
  lib/RPSTypes.cpp
)

target_include_directories(
  ngl-tbc-samples-rps_lib ${warning_guard}
  PUBLIC "\$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/>"
)

target_compile_features(
  ngl-tbc_lib 
  PUBLIC cxx_std_20
)

find_package(fmt REQUIRED)
find_package(Threads REQUIRED)
target_link_libraries(
  ngl-tbc-samples-rps_lib
  PUBLIC ngl-tbc_lib
  PRIVATE fmt::fmt
  PRIVATE Threads::Threads
)

add_executable(
  ngl-tbc-samples-rps-cli_exe cli_app.cpp
)

target_compile_features(ngl-tbc-samples-rps-cli_exe PRIVATE cxx_std_20)

target_link_libraries(
  ngl-tbc-samples-rps-cli_exe 
  PUBLIC ngl-tbc_lib
  PUBLIC ngl-tbc-samples-rps_lib
)

target_include_directories(
  ngl-tbc-samples-rps-cli_exe 
  PUBLIC "\$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/>"
)