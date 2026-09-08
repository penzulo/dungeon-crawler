cmake_minimum_required(VERSION 3.25)
project(dungeon-crawler LANGUAGES CXX)

add_compile_options(-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Werror)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

enable_testing()
add_executable(dungeon-crawler src/main.cpp)
