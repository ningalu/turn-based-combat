add_library(
    ningalu-turn-based-combat_lib OBJECT
    
    lib.cpp
)

target_include_directories(
    ningalu-turn-based-combat_lib ${warning_guard}
    PUBLIC
    "\$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/>"
)

target_compile_features(ningalu-turn-based-combat_lib PUBLIC cxx_std_20)

find_package(fmt REQUIRED)
target_link_libraries(ningalu-turn-based-combat_lib PRIVATE fmt::fmt)