export module dungeon.room;
import std;
import dungeon.monster;
import dungeon.item;

export using std::string, std::optional, std::array;

export constexpr uint8_t num_exits{4};

export enum class Direction : uint8_t { North, East, South, West };

export constexpr Direction opposite(const Direction d) noexcept {
  switch (d) {
    case Direction::North: return Direction::South;
    case Direction::South: return Direction::North;
    case Direction::East: return Direction::West;
    case Direction::West: return Direction::East;
  }
  std::unreachable();
}

/*
 * Room has a fixed array (size 4) of exits which are
 * optional values of `uint8_t`. Subscripting the exits
 * array by the `Direction` enum is a design choice.
 */
export struct Room {
  string name;
  string description;

  // subscripted by the direction enum
  array<optional<uint8_t>, num_exits> exits;

  optional<Monster> monster;
  optional<Item> room_item;

  auto exit_at(const Direction d) const noexcept -> optional<uint8_t> {
    return exits[std::to_underlying(d)];
  }

  auto release_item() -> optional<Item> { return std::exchange(room_item, std::nullopt); }

  auto release_monster() -> optional<Monster> { return std::exchange(monster, std::nullopt); }

  auto has_item() const -> bool { return room_item.has_value(); }
  auto has_monster() const -> bool { return monster.has_value(); }
};
