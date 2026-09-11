# Dungeon Crawler — dev command runner

build_dir := "build"
target := "dungeon-crawler"

default:
    @just --list

# Configure the build (Ninja + clang++ + libc++ via CMakePresets.json).
configure:
    cmake --preset default

# Build all targets. (configure silently if needed)
build: configure
    cmake --build {{build_dir}} --parallel

alias b := build

# Run the game executable.
run:
    ./{{build_dir}}/{{target}}

alias r := run

# Configure + build + run in one go.
dev: build
    ./{{build_dir}}/{{target}}

# Symlink compile_commands.json to the project root for clangd.
index: build
    ln -sfn {{build_dir}}/compile_commands.json compile_commands.json

# Format all sources with clang-format.
format:
    clang-format -i src/*.cppm src/main.cpp

# Check formatting without modifying files.
check:
    clang-format --dry-run --Werror src/*.cppm src/main.cpp

# Build and run the test suite.
test:
    ctest --test-dir {{build_dir}} --output-on-failure

# Remove build artifacts.
clean:
    cmake -E remove_directory {{build_dir}}
    rm -f compile_commands.json
