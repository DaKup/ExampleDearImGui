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

add_executable(ExampleDearImGui_exe
  source/glfw_vulkan.cpp
  source/application.cpp
  
  # dear imgui files:
  # backends:
  imgui/backends/imgui_impl_glfw.cpp
  imgui/backends/imgui_impl_vulkan.cpp

  # imgui:
  imgui/imgui.cpp
  imgui/imgui_draw.cpp
  imgui/imgui_demo.cpp
  
  imgui/imgui_tables.cpp
  imgui/imgui_widgets.cpp

  # headers (for windows):
  imgui/imgui.h
  imgui/imgui_internal.h
  imgui/imstb_rectpack.h
  imgui/imstb_textedit.h
  imgui/imstb_truetype.h
  )
add_executable(ExampleDearImGui::exe ALIAS ExampleDearImGui_exe)

set_target_properties(
    ExampleDearImGui_exe PROPERTIES
    OUTPUT_NAME ExampleDearImGui
    EXPORT_NAME exe
)

target_include_directories(
  ExampleDearImGui_exe ${warning_guard}
    PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/imgui>"
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/imgui/backends>"
)

target_compile_features(ExampleDearImGui_exe PRIVATE cxx_std_17)

find_package(Vulkan REQUIRED)
find_package(glm REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glslang CONFIG REQUIRED)

target_link_libraries(ExampleDearImGui_exe PRIVATE ExampleDearImGui_lib glfw Vulkan::Vulkan glm::glm glslang)
target_compile_definitions(ExampleDearImGui_exe PUBLIC -DImTextureID=ImU64) # required for user textures in vulkan

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
