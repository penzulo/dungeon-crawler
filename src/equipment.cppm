export module dungeon.equipment;
import std;

using std::optional, std::uint8_t;

// Items which are currently being used.
export struct Equipment {
  optional<uint8_t> equipped_weapon;

  Equipment() = default;
};
