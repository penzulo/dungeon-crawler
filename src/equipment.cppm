export module dungeon.equipment;
import std;

// Items which are currently being used.
export struct Equipment {
  std::optional<std::size_t> equipped_weapon;

  Equipment() = default;
};