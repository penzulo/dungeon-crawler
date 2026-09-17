import boost.ut;
import dungeon.game;
import dungeon.health;
import dungeon.monster;
import dungeon.item;

using namespace boost::ut;

suite<"game_flow"> game_flow_suite = [] {
  "exit quits the game"_test = [] {
    Game game{.dungeon = make_dungeon(), .hero = Character{.name = "Tester"}};

    const auto result = game.next_state({CommandType::Exit, std::nullopt});

    expect(result.has_value());
    expect(eq(result->state, GameState::Quit));
    expect(eq(game.state, GameState::Quit));
  };
};

suite<"combat"> combat_suite = [] {
  "cannot attack non-existent monster in empty room"_test = [] {
    Game game{.dungeon = make_dungeon(), .hero = Character{.name = "Tester"}};

    const auto result = game.next_state({CommandType::Attack, std::nullopt});

    expect(!result.has_value());
    expect(eq(result.error(), GameError::NothingToFight));
    expect(eq(game.state, GameState::Playing));
  };

  "attacking a 1 HP monster slays it"_test = [] {
    Game game{.dungeon = make_dungeon(), .hero = Character{.name = "Tester"}};
    auto& room = game.dungeon.current_room();
    room.monster = Monster{MonsterType::Goblin};
    room.monster->health.current = 1;

    const auto result = game.next_state({.command = CommandType::Attack, .slot = std::nullopt});

    expect(result.has_value());
    expect(eq(result->state, GameState::Playing));
    expect(eq(game.state, GameState::Playing));
    expect(!room.has_monster());
    expect(result->narration.find("slew") != std::string::npos);
  };

  "attacking a full health monster does not kill it"_test = [] {
    Game game{.dungeon = make_dungeon(), .hero = Character{.name = "Tester"}};
    game.dungeon.enter_room(1);
    auto& room = game.dungeon.current_room();

    const auto result = game.next_state({.command = CommandType::Attack, .slot = std::nullopt});

    expect(result.has_value());
    expect(room.has_monster());
    expect(eq(room.monster->health.current,
              stats(MonsterType::Goblin).max_health - game.hero.attack_power()));
    expect(eq(result->state, GameState::Playing));
    expect(eq(game.state, GameState::Playing));
  };

  "hero dies on low health"_test = [] {
    Game game{.dungeon = make_dungeon(), .hero = Character{.name = "Tester"}};
    game.hero.health.current = 3;
    game.dungeon.enter_room(1);

    const auto result = game.next_state({.command = CommandType::Attack, .slot = std::nullopt});

    expect(game.hero.is_dead());
    expect(eq(game.dungeon.current_room().monster->health.current,
              stats(MonsterType::Goblin).max_health - game.hero.attack_power()));
    expect(eq(result->state, GameState::Dead));
    expect(eq(game.state, GameState::Dead));
    expect(result->narration.find("died") != std::string::npos);
  };

  "monster dies when both on same health"_test = [] {
    Game game{.dungeon = make_dungeon(), .hero = Character{.name = "Tester"}};
    game.dungeon.enter_room(1);
    game.hero.health.current = 2;
    game.dungeon.current_room().monster->health.current = 2;

    const auto result = game.next_state({.command = CommandType::Attack, .slot = std::nullopt});

    expect(!game.hero.is_dead());
    expect(!game.dungeon.current_room().has_monster());
    expect(eq(game.hero.health.current, 2));
    expect(eq(result->state, GameState::Playing));
    expect(eq(game.state, GameState::Playing));
  };

  "attacking a dead monster releases it without error"_test = [] {
    Game game{.dungeon = make_dungeon(), .hero = Character{.name = "Tester"}};
    auto& room = game.dungeon.current_room();
    room.monster = Monster{MonsterType::Goblin};
    room.monster->health.current = 0;

    const auto result = game.next_state({.command = CommandType::Attack, .slot = std::nullopt});

    expect(result.has_value());
    expect(!room.has_monster());  // combat_round's or_else releases it, not NothingToFight
    expect(eq(result->state, GameState::Playing));
  };
};

suite<"items"> items_suite = [] {
  "cannot pick up item from empty room"_test = [] {
    Game game{.dungeon = make_dungeon(), .hero = Character{.name = "Tester"}};
    game.dungeon.current_room().room_item = std::nullopt;

    const auto result = game.next_state({.command = CommandType::PickUp, .slot = 0});

    expect(!result.has_value());
    expect(eq(result.error(), GameError::NoItemOnFloor));
  };

  "cannot pick up item from guarded room"_test = [] {
    Game game{.dungeon = make_dungeon(), .hero = Character{.name = "Tester"}};
    game.dungeon.enter_room(1);

    const auto result = game.next_state({.command = CommandType::PickUp, .slot = 0});

    expect(!result.has_value());
    expect(eq(result.error(), GameError::GuardedByMonster));
    expect(eq(game.state, GameState::Playing));
  };

  "item can be picked up when monster is dead"_test = [] {
    Game game{.dungeon = make_dungeon(), .hero = Character{.name = "Tester"}};
    game.dungeon.enter_room(1);
    const auto& room = game.dungeon.current_room();

    while (room.has_monster() && room.monster->is_alive()) {
      game.next_state({.command = CommandType::Attack, .slot = 0});
    }

    const auto result = game.next_state({.command = CommandType::PickUp, .slot = 0});
    expect(!room.has_monster());
    expect(result.has_value());
    expect(eq(result->state, GameState::Playing));
    expect(eq(game.state, GameState::Playing));
  };

  "arbitrary slot not permitted"_test = [] {
    Game game{.dungeon = make_dungeon(), .hero = Character{.name = "Tester"}};

    const auto result = game.next_state({.command = CommandType::PickUp, .slot = 69});

    expect(!result.has_value());
    expect(eq(result.error(), GameError::NoSuchSlot));
    expect(eq(game.state, GameState::Playing));
  };

  "cannot pick up into an occupied slot"_test = [] {
    Game game{.dungeon = make_dungeon(), .hero = Character{.name = "Tester"}};
    game.hero.inventory.items[0] = Item{"Junk", ItemType::Consumable, 1};
    game.dungeon.current_room().room_item = Item{"Healing Draught", ItemType::Consumable, 15};

    const auto result = game.next_state({.command = CommandType::PickUp, .slot = 0});

    expect(!result.has_value());
    expect(eq(result.error(), GameError::SlotOccupied));
    expect(eq(game.state, GameState::Playing));
  };
};

suite<"inventory"> inventory_suite = [] {
  "adding quest item to inventory ends the game"_test = [] {
    Game game{.dungeon = make_dungeon(), .hero = Character{.name = "Tester"}};
    game.dungeon.rooms[0].room_item = Item("Ancient necklace", ItemType::QuestItem, 0);

    const auto result = game.next_state({CommandType::PickUp, 0});

    expect(result.has_value());
    expect(eq(result->state, GameState::Won));
    expect(eq(game.state, GameState::Won));
    expect(game.hero.inventory.items[0].has_value());
  };
};

suite<"directions"> direction_suite = [] {
  "valid direction changes player position"_test = [] {
    Game game{.dungeon = make_dungeon(), .hero = Character{.name = "Tester"}};

    // Should go to the next room
    const auto result = game.next_state({.command = CommandType::East, .slot = std::nullopt});

    expect(result.has_value());
    expect(eq(game.dungeon.player_pos, 1));
    expect(eq(result->state, GameState::Playing));
    expect(eq(game.state, GameState::Playing));
  };

  "move in a non-exit direction is not permitted"_test = [] {
    Game game{.dungeon = make_dungeon(), .hero = Character{.name = "Tester"}};

    // Any direction except east in the first room is invalid
    const auto result = game.next_state({.command = CommandType::North, .slot = std::nullopt});

    expect(!result.has_value());
    expect(eq(result.error(), GameError::NotAnExit));
    expect(eq(game.state, GameState::Playing));
  };
};

suite<"use"> use_suite = [] {
  "use without a slot is rejected"_test = [] {
    Game game{.dungeon = make_dungeon(), .hero = Character{.name = "Tester"}};
    const auto result = game.next_state({CommandType::Use, std::nullopt});
    expect(!result.has_value());
    expect(eq(result.error(), GameError::NoSuchSlot));
  };

  "use of an out-of-range slot is rejected"_test = [] {
    Game game{.dungeon = make_dungeon(), .hero = Character{.name = "Tester"}};
    const auto result = game.next_state({CommandType::Use, 69});
    expect(!result.has_value());
    expect(eq(result.error(), GameError::NoSuchSlot));
  };

  "use of an empty slot is rejected"_test = [] {
    Game game{.dungeon = make_dungeon(), .hero = Character{.name = "Tester"}};
    const auto result = game.next_state({CommandType::Use, 0});
    expect(eq(result.error(), GameError::EmptySlot));
  };

  "cannot consume at full health"_test = [] {
    Game game{.dungeon = make_dungeon(), .hero = Character{.name = "Tester"}};
    game.hero.inventory.items[0] = Item{"Healing Draught", ItemType::Consumable, 15};
    const auto result = game.next_state({CommandType::Use, 0});  // hero is full 100/100
    expect(eq(result.error(), GameError::AlreadyAtFullHealth));
    expect(eq(game.state, GameState::Playing));
  };

  "cannot use a quest item directly"_test = [] {
    Game game{.dungeon = make_dungeon(), .hero = Character{.name = "Tester"}};
    game.hero.inventory.items[0] = Item{"Ancient Pendant", ItemType::QuestItem, 0};
    const auto result = game.next_state({CommandType::Use, 0});
    expect(eq(result.error(), GameError::CannotUseDirectly));
  };

  "using a consumable heals and consumes it"_test = [] {
    Game game{.dungeon = make_dungeon(), .hero = Character{.name = "Tester"}};
    game.hero.health.current = 50;
    game.hero.inventory.items[0] = Item{"Healing Draught", ItemType::Consumable, 15};
    const auto result = game.next_state({CommandType::Use, 0});
    expect(result.has_value());
    expect(eq(game.hero.health.current, 65));
    expect(!game.hero.inventory.items[0].has_value());  // consumed
    expect(result->narration.find("Healing Draught") != std::string::npos);
  };

  "equipping a weapon raises attack power"_test = [] {
    Game game{.dungeon = make_dungeon(), .hero = Character{.name = "Tester"}};
    game.hero.inventory.items[0] = Item{"Rusty Sword", ItemType::Weapon, 5};
    const auto result = game.next_state({CommandType::Use, 0});
    expect(result.has_value());
    expect(game.hero.equipment.equipped_weapon.has_value());
    expect(eq(game.hero.attack_power(), 5));  // was base_attack 2
  };
};

int main() { return 0; }
