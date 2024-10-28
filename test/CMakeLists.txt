# Parent project does not export its library target, so this CML implicitly
# depends on being added from it, i.e. the testing is done only from the build
# tree and is not feasible from an install location

project(ngl-tbc-2Tests LANGUAGES CXX)

# ---- Dependencies ----

find_package(Catch2 REQUIRED)
include(Catch)

# ---- Tests ----

add_executable(
    ngl-tbc-2_test 
    source/tbc/EventHandlerTest.cpp
    source/tbc/RPGLayoutTest.cpp
    source/tbc/LogTest.cpp
)

target_link_libraries(
    ngl-tbc-2_test 
    PUBLIC ngl-tbc_lib
    PUBLIC ngl-tbc-samples-nac_lib
    PUBLIC ngl-tbc-samples-rps_lib
    Catch2::Catch2WithMain
)
target_compile_features(ngl-tbc-2_test PRIVATE cxx_std_20)

add_executable(
    ngl-tbc-nac_test

    source/samples/NoughtsAndCrosses/NACTypesTest.cpp
    source/samples/NoughtsAndCrosses/NACIntegrationTest.cpp
)

target_link_libraries(
    ngl-tbc-nac_test 
    PUBLIC ngl-tbc_lib
    PUBLIC ngl-tbc-samples-nac_lib
    Catch2::Catch2WithMain
)
target_compile_features(
    ngl-tbc-nac_test 
    PRIVATE cxx_std_20
)

add_executable(
    ngl-tbc-rps_test

    source/samples/RockPaperScissors/RPSIntegrationTest.cpp
)

target_link_libraries(
    ngl-tbc-rps_test 
    PUBLIC ngl-tbc_lib
    PUBLIC ngl-tbc-samples-rps_lib
    Catch2::Catch2WithMain
)
target_compile_features(
    ngl-tbc-rps_test 
    PRIVATE cxx_std_20
)

catch_discover_tests(ngl-tbc-2_test)
catch_discover_tests(ngl-tbc-nac_test)
catch_discover_tests(ngl-tbc-rps_test)

# ---- End-of-file commands ----

add_folders(Test)
