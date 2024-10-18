install(
    TARGETS ngl-tbc-2_exe
    RUNTIME COMPONENT ngl-tbc-2_Runtime
)

if(PROJECT_IS_TOP_LEVEL)
  include(CPack)
endif()
