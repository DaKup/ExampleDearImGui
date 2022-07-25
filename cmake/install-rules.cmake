install(
    TARGETS ExampleDearImGui_exe
    RUNTIME COMPONENT ExampleDearImGui_Runtime
)

if(PROJECT_IS_TOP_LEVEL)
  include(CPack)
endif()
