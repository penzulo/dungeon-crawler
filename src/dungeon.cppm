export module dungeon.dungeon;
import std;
import dungeon.character;
import dungeon.health;
import dungeon.room;
import dungeon.monster;
import dungeon.item;

export using std::uint8_t, std::expected, std::unexpected, std::string_view;

export constexpr uint8_t n_rooms{5};

export enum MovementError {
  NotAnExit,  // Thrown when there is no exit in the direction
};

export enum CombatError {
  NothingToFight  // Cannot fight a dead or non-existing monster
};

export struct CombatResult {
  uint8_t dealt{};
  uint8_t taken{};
  bool foe_slain{false};
};

/*
 * Needed so that the loop can say what it sees.
 */
export auto name(const Direction d) -> string_view {
  switch (d) {
    case Direction::North: return "North";
    case Direction::East: return "East";
    case Direction::South: return "South";
    case Direction::West: return "West";
  }
  std::unreachable();
}

export auto combat_round(Character& player, Room& room) -> expected<CombatResult, CombatError> {
  if (!room.has_monster()) {
    return unexpected(CombatError::NothingToFight);
  }

  Monster& mon = *room.monster;

  return player.attack(mon)
      .and_then([&](const uint8_t dealt) -> expected<CombatResult, HealthError> {
        if (mon.is_dead()) {  // This blow killed it
          room.release_monster();
          return CombatResult{.dealt = dealt, .foe_slain = true};
        }

        const auto taken = player.health.take_damage(stats(mon.type).damage);

        return CombatResult{.dealt = dealt, .taken = taken.value_or(0)};
      })
      .or_else([&](const HealthError) -> expected<CombatResult, CombatError> {
        room.release_monster();
        return CombatResult{};
      });
}

export struct Dungeon {
  std::array<Room, n_rooms> rooms;
  uint8_t player_pos{};

  auto current_room_index() const noexcept -> uint8_t { return player_pos; }
  auto current_room() noexcept -> Room& { return rooms[player_pos]; }
  auto current_room() const noexcept -> const Room& { return rooms[player_pos]; }

  auto move(const Direction d) -> expected<void, MovementError> {
    // Does the room in Direction `d` from the current room exist?
    // NOTE: optional cannot be used with expected<T>
    const auto next_room = current_room().exit_at(d);

    if (!next_room) {
      return unexpected(MovementError::NotAnExit);
    }

    player_pos = next_room.value();
    return {};
  }

  auto connect(const uint8_t a, const Direction d, const uint8_t b) -> void {
    // assert(a < n_rooms);
    // assert(b < n_rooms);
    // assert(a != b);

    rooms[a].exits[std::to_underlying(d)] = b;
    rooms[b].exits[std::to_underlying(opposite(d))] = a;
  }

  auto disconnect(const uint8_t a, const Direction d, const uint8_t b) -> void {
    // assert(a < n_rooms);
    // assert(b < n_rooms);
    // assert(a != b);

    rooms[a].exits[std::to_underlying(d)].reset();
    rooms[b].exits[std::to_underlying(opposite(d))].reset();
  }

  auto enter_room(const uint8_t index) -> void {
    // assert(index < n_rooms);
    player_pos = index;
  }
};

export auto make_dungeon() -> Dungeon {
  Dungeon d;

  d.rooms[0].name = "Entrance Hall";
  d.rooms[0].description = "A dusty hall beneath the old fortress gate.";
  d.rooms[0].room_item = Item{"Healing Draught", ItemType::Consumable, 15};

  d.rooms[1].name = "Barracks";
  d.rooms[1].description = "Rotting bunks line the walls, long abandoned.";
  d.rooms[1].monster = Monster{MonsterType::Goblin};
  d.rooms[1].room_item = Item{"Stale Rations", ItemType::Consumable, 10};

  d.rooms[2].name = "Armory";
  d.rooms[2].description = "Weapons racks stand stripped and rusted.";
  d.rooms[2].monster = Monster{MonsterType::Wolf};
  d.rooms[2].room_item = Item{"Rusty Sword", ItemType::Weapon, 5};

  d.rooms[3].name = "Old Courtyard";
  d.rooms[3].description = "Rain has found its way through the broken roof.";
  d.rooms[3].monster = Monster{MonsterType::Skeleton};
  d.rooms[3].room_item = Item{"Healing Draught", ItemType::Consumable, 15};

  d.rooms[4].name = "Throne Room";
  d.rooms[4].description = "The source of the disturbance lurks here.";
  d.rooms[4].monster = Monster{MonsterType::Troll};
  d.rooms[4].room_item = Item{"Ancient Pendant", ItemType::QuestItem, 0};

  // The dungeon is a straight line heading east.
  d.connect(0, Direction::East, 1);
  d.connect(1, Direction::East, 2);
  d.connect(2, Direction::East, 3);
  d.connect(3, Direction::East, 4);

  d.enter_room(0);

  return d;
}
