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
    source/tbc/LayoutTest.cpp
    source/tbc/TargetTest.cpp
    source/tbc/LogTest.cpp
)

target_link_libraries(
    ngl-tbc-2_test PRIVATE
    ngl-tbc_lib
    Catch2::Catch2WithMain
)
target_compile_features(ngl-tbc-2_test PRIVATE cxx_std_20)

catch_discover_tests(ngl-tbc-2_test)

# ---- End-of-file commands ----

add_folders(Test)
