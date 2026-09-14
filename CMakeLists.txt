cmake_minimum_required(VERSION 3.28)

set(CMAKE_EXPERIMENTAL_CXX_MODULE_DYNDEP "8578c1b2-a62a-4a2d-890a-0d3c5619e7d8")

project(dungeon-crawler LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Werror)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-stdlib=libc++)
  add_link_options(-stdlib=libc++)
else()
  message(FATAL_ERROR
    "dungeon-crawler requires Clang (clang++) — the std module needs libc++.")
endif()

enable_testing()

# Precompile the C++23 standard library module (`import std;`).
# CMake's experimental `import std` support is avoided on purpose: this is
# explicit, works without extra gates, and puts the BMI path into
# compile_commands.json so clangd resolves `import std;` too.
set(STD_MODULE_SRC "/usr/share/libc++/v1/std.cppm")
set(STD_PCM "${CMAKE_BINARY_DIR}/modules/std.pcm")

add_custom_command(
  OUTPUT "${STD_PCM}"
  COMMAND "${CMAKE_COMMAND}" -E make_directory "${CMAKE_BINARY_DIR}/modules"
  COMMAND "${CMAKE_CXX_COMPILER}" -std=c++23 -stdlib=libc++
          -Wno-reserved-module-identifier -x c++-module --precompile
          -o "${STD_PCM}" "${STD_MODULE_SRC}"
  DEPENDS "${STD_MODULE_SRC}"
  COMMENT "Precompiling std module → ${STD_PCM}"
  VERBATIM)

add_custom_target(std-module DEPENDS "${STD_PCM}")
add_compile_options("-fmodule-file=std=${STD_PCM}")

add_library(dungeon-core)
target_sources(dungeon-core
  PUBLIC
    FILE_SET CXX_MODULES
    FILES
      src/health.cppm
      src/item.cppm
      src/inventory.cppm
      src/equipment.cppm
      src/command.cppm
      src/character.cppm
      src/monster.cppm
      src/room.cppm
      src/dungeon.cppm
      src/game.cppm
      )

add_executable(dungeon-crawler src/main.cpp)
target_link_libraries(dungeon-crawler PRIVATE dungeon-core)

add_dependencies(dungeon-core std-module)
add_dependencies(dungeon-crawler std-module)
