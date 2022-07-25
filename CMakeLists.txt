cmake_minimum_required(VERSION 3.14)

set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake CACHE STRING "Vcpkg toolchain file")

include(cmake/prelude.cmake)

project(
    ExampleDearImGui
    VERSION 0.1.0
    DESCRIPTION "Short description"
    HOMEPAGE_URL "https://example.com/"
    LANGUAGES CXX
)

include(cmake/project-is-top-level.cmake)
include(cmake/variables.cmake)

# ---- Declare library ----

add_library(
    ExampleDearImGui_lib OBJECT
    source/lib.cpp
)

target_include_directories(
    ExampleDearImGui_lib ${warning_guard}
    PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/source>"
)

target_compile_features(ExampleDearImGui_lib PUBLIC cxx_std_17)

find_package(fmt REQUIRED)
target_link_libraries(ExampleDearImGui_lib PRIVATE fmt::fmt)

# ---- Declare executable ----

add_executable(ExampleDearImGui_exe source/main.cpp)
add_executable(ExampleDearImGui::exe ALIAS ExampleDearImGui_exe)

set_target_properties(
    ExampleDearImGui_exe PROPERTIES
    OUTPUT_NAME ExampleDearImGui
    EXPORT_NAME exe
)

target_compile_features(ExampleDearImGui_exe PRIVATE cxx_std_17)

target_link_libraries(ExampleDearImGui_exe PRIVATE ExampleDearImGui_lib)

# ---- Install rules ----

if(NOT CMAKE_SKIP_INSTALL_RULES)
  include(cmake/install-rules.cmake)
endif()

# ---- Developer mode ----

if(NOT ExampleDearImGui_DEVELOPER_MODE)
  return()
elseif(NOT PROJECT_IS_TOP_LEVEL)
  message(
      AUTHOR_WARNING
      "Developer mode is intended for developers of ExampleDearImGui"
  )
endif()

include(cmake/dev-mode.cmake)
