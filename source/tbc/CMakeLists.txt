add_library(
    ngl-tbc_lib OBJECT
    
    Target.cpp

    Slot.cpp
    Side.cpp
    Layout.cpp
    Battle.cpp
)

target_include_directories(
    ngl-tbc_lib ${warning_guard}
    PUBLIC "\$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/>"
)

target_compile_features(ngl-tbc_lib PUBLIC cxx_std_20)

find_package(fmt REQUIRED)
target_link_libraries(ngl-tbc_lib PRIVATE fmt::fmt)