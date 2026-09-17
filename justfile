# Dungeon Crawler — development command runner.
#
# Run `just` with no arguments to list every recipe.

# Fail fast, and treat unset variables / broken pipes as errors.
set shell := ["bash", "-euo", "pipefail", "-c"]

# Directory the preset builds into, and the primary executable name.
build_dir := "build"
target := "dungeon-crawler"

# Every C++ source covered by the formatting gate.
sources := "src/*.cppm src/main.cpp tests/*.cpp"

# ---------------------------------------------------------------------------
# meta
# ---------------------------------------------------------------------------

# List all available recipes.
[private]
default:
    @just --list

# ---------------------------------------------------------------------------
# build
# ---------------------------------------------------------------------------

# Configure the CMake build (Ninja + clang++ + libc++ via CMakePresets.json).
[group('build')]
configure:
    cmake --preset default

# Build every target, configuring first if needed.
[group('build')]
build: configure
    cmake --build {{build_dir}} --parallel

alias b := build

# Delete the build directory and the clangd symlink.
[group('build')]
[confirm('Remove the build directory and compile_commands.json?')]
clean:
    cmake -E remove_directory {{build_dir}}
    rm -f compile_commands.json

# ---------------------------------------------------------------------------
# run
# ---------------------------------------------------------------------------

# Run the game.
[group('run')]
run:
    ./{{build_dir}}/{{target}}

alias r := run

# Build, then run the game.
[group('run')]
dev: build
    ./{{build_dir}}/{{target}}

# ---------------------------------------------------------------------------
# quality
# ---------------------------------------------------------------------------

# Format all C++ sources in place.
[group('quality')]
format:
    clang-format -i {{sources}}

alias f := format

# Verify formatting without touching files.
[group('quality')]
check:
    clang-format --dry-run --Werror {{sources}}

# Build and run the test suite.
[group('quality')]
test: build
    ctest --test-dir {{build_dir}} --output-on-failure

alias t := test

# ---------------------------------------------------------------------------
# tooling
# ---------------------------------------------------------------------------

# Symlink compile_commands.json into the project root for clangd.
[group('tooling')]
index: build
    ln -sfn {{build_dir}}/compile_commands.json compile_commands.json

alias i := index
