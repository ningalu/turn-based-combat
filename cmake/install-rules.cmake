install(
    TARGETS ningalu-turn-based-combat-2_exe
    RUNTIME COMPONENT ningalu-turn-based-combat-2_Runtime
)

if(PROJECT_IS_TOP_LEVEL)
  include(CPack)
endif()
