# Dungeon Crawler

A small dungeon crawler written from scratch in **C++23** as a *learning
project*. The whole point of this repository is to teach, not to ship: every
concept is split into tiny, readable pieces, and a deliberate pile of fun
features is still waiting to be written (see [What's left](#whats-left-the-fun-part)).

If you are learning modern C++, this repo is an honest record of someone doing
exactly that — including the stumbles. The code compiles strictly
(`-Werror`), the design decisions are documented, and the gap between "it
runs" and "it's a game you'd want to play" is wide open on purpose.

## What this project is exploring

- **C++20/23 modules** — one `.cppm` per concept, explicitly exported
  interfaces, private-by-default names. No more header soup.
- **`import std;`** — the standard library as a module, precompiled once via
  CMake and shared by every module unit.
- **`std::expected` and monadic error handling** — combat and movement are
  chained with `and_then` / `or_else` instead of manual error branches.
- **Strict compilation** — `-Wall -Wextra -Wpedantic -Wshadow -Wconversion
  -Werror` are the baseline; the build is green or it doesn't exist.
- **CMake's experimental module support** — precompiling the `std` module
  explicitly so `clangd` can resolve `import std;` too.

## Playable today

The current game is a text loop: the room is described, you type a command,
the world reacts, and (every step) your hero, inventory, room, monsters,
items and exits are printed again.

The dungeon is a straight line of five rooms heading east:

| Room | Rooms & contents |
|---|---|
| Entrance Hall | a Healing Draught on the floor |
| Barracks | a **Goblin** (3 dmg / 20 HP) and stale rations |
| Armory | a **Wolf** (5 dmg / 25 HP) and the Rusty Sword (5 dmg) |
| Old Courtyard | a **Skeleton** (4 dmg / 35 HP) and another draught |
| Throne Room | a **Troll** (8 dmg / 50 HP) and the Ancient Pendant |

### Commands

| Command | Does |
|---|---|
| `n` / `e` / `s` / `w` | move North / East / South / West |
| `attack` | fight the monster in the room |
| `pickup <slot>` | take the item on the floor into inventory slot `<slot>` |
| `use <slot>` | drink a draught or equip a weapon from slot `<slot>` |
| `exit` | give up and leave the dungeon |

### Bravely honest caveats

- **The win path is currently unreachable by balance.** The Rusty Sword sits
  *behind* the Wolf, so the Wolf must be fought bare-handed; the total damage
  the dungeon deals (≈160–220 HP) overwhelms the healing available (≈140 HP).
  The *code* for victory exists — it's just numbers that need tuning. Death,
  on the other hand, definitely works.
- There are **no unit tests yet** — Slice 4 (boost-ext/ut) is the next
  planned slice.

## What's left (the fun part)

All of this is genuinely still on the table. The game beating you into the
ground is fine; it's a learning sandbox, and the list below is the queue:

- [ ] **Saving the game state to disk** — serialize the dungeon, hero,
      inventory, and quest progress so you can put the game down.
- [ ] **Loading the game state from disk** — and continuing where you left
      off.
- [x] **Decoupling the game-running loop from `main`** — done (Slice 3,
      commit `c2571c6`). `main` reads input and prints; `Game::next_state`
      owns the turn logic.
- [ ] **Logging to a file** — an audit trail of every turn (and a great
      excuse to learn about sinks, timestamps, and ring buffers).
- [ ] **Arrow-key controls** — ↑ for North, ← for West, ↓ for South, → for
      East, via raw terminal input instead of typed words.
- [ ] **Colorized text** — ANSI colors for rooms, monsters, and damage.
- [ ] **A TUI-like fixed screen** — split the display into fixed panes:
      inventory pinned on the right, room description + monster on the main
      screen, equipment in its own fixed spot.
- [ ] **Redraw-in-place rendering** — each game loop pass rewrites the old
      text in place, so the stdout buffer stays the same size and the screen
      never scrolls.
- [ ] **Correct combat balance** — move the sword in front of the Wolf (or
      weaken the Wolf/Troll, or add healing), then verify the full win path:
      Goblin → Wolf → Skeleton → Troll → pendant → victory.
- [ ] **Unit tests** — next up (boost-ext/ut, module-native). `next_state`
      is a clean seam: `Game` + `UserIntent` → `expected<TurnResult,
      GameError>`. Must include a regression test for the `Game::state` sync
      bug (§3.11 in TODO.md).

## Module layout

| Module | Exports |
|---|---|
| `dungeon.health` | `Health`, `HealthError` |
| `dungeon.item` | `Item`, `ItemType`, `name` |
| `dungeon.inventory` | `Inventory`, `InventoryError`, `ItemRef`, `inventory_capacity` |
| `dungeon.equipment` | `Equipment` |
| `dungeon.command` | `CommandType`, `to_direction`, `parse_command`, `parse_slot` |
| `dungeon.character` | `Character`, `UseItemError`, `attack`, `pick_item`, `use_item`, `attack_power` |
| `dungeon.monster` | `Monster`, `MonsterType`, `MonsterStats`, `stats`, `name` |
| `dungeon.room` | `Room`, `Direction`, `exit_at`, `release_item`, `release_monster`, `has_item`, `has_monster` |
| `dungeon.dungeon` | `Dungeon`, `combat_round`, `make_dungeon`, `name` |
| `dungeon.game` | `GameState`, `GameError`, `TurnResult`, `UserIntent`, `Game`, `describe`, `error_message` |
| `main.cpp` | thin driver — reads input, prints output (game logic lives in `dungeon.game`) |

One pacing rule keeps the module graph acyclic: `dungeon.character` never
imports `dungeon.room` or `dungeon.dungeon`. Combat coordination — which has
to bridge the character and the room — lives in `dungeon.dungeon`, which may
import `dungeon.character`.

## Gotchas learned so far

Write-ups are worth more than wins, so here are the pitfalls this project
already tripped over:

- `assert` is a **macro**, so it is not part of `import std;` (unlike most
  of `<cassert>`'s siblings, it was deliberately excluded). The fix is a
  *global module fragment* at the top of the module:
  ```cpp
  module;
  #include <cassert>
  export module dungeon.dungeon;
  ```
  A plain `#include <cassert>` inside the module body fails
  `-Winclude-angled-in-module-purview` under `-Werror`.
- Modules **don't leak `using` declarations** — every `.cppm` declares its
  own.
- Name types explicitly: `std::size_t`, `std::optional`, `std::expected` —
  nothing sneaks in from a parent header.
- ~~`std::println` stays in `main.cpp`; module units just hand data back.~~
  This rule was relaxed in Slice 3: `dungeon.game` now owns `describe` and
  `error_message`, which print directly. The goal is a single import
  (`import dungeon.game;`) for consumers.

## Building and running

Requirements:

- CMake ≥ 3.28 (for module support)
- a C++23 compiler with modules + **libc++** — this repo is set up for
  clang++ (`clang++` + `libc++` are forced in the presets)
- Ninja
- [`just`](https://github.com/casey/just) (optional — you can run the
  underlying cmake commands yourself)

Then:

```bash
just dev                      # configure + build + run
```

Or without `just`:

```bash
cmake --preset default
cmake --build build --parallel
./build/dungeon-crawler
```

The `justfile` is the command runner:

| Command | Does |
|---|---|
| `just configure` | run `cmake --preset default` |
| `just build` / `just b` | build everything (configures if needed) |
| `just run` / `just r` | run the game |
| `just dev` | build and run in one go |
| `just index` | symlink `compile_commands.json` for `clangd` |
| `just format` | clang-format all sources in place |
| `just check` | verify formatting without touching files (`:v`-flag) |
| `just test` / `just t` | build and run the test suite via `ctest` |
| `just clean` | wipe build artifacts (asks for confirmation) |

## Quality gates

- Strict warnings as errors on every translation unit (`-Werror`).
- `just check` runs `clang-format --dry-run --Werror` over every `.cppm` and
  `main.cpp` — unmatched formatting fails the gate.
- The `std` module is precompiled as `build/modules/std.pcm` and consumed via
  `-fmodule-file=std=<pcm>`, so the exact `import std;` the compiler uses is
  visible in `compile_commands.json` for `clangd`.