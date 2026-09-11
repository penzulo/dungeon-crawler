import std;
import dungeon.dungeon;
import dungeon.command;
import dungeon.character;
import dungeon.inventory;
import dungeon.room;
import dungeon.monster;
import dungeon.item;

using std::optional, std::string;

auto describe(const Dungeon& dun, const Character& player) {
  const auto& room = dun.current_room();
  const auto weapon = player.equipment.equipped_weapon.and_then([&player](const uint8_t index) {
    const auto res = player.inventory.get_item(index);
    return res ? optional{res.value()} : std::nullopt;
  });

  // 1. Hero Line
  std::println("{} — HP {}/{}", player.name, player.health.current, player.health.maximum);

  // 2. Equipment information
  std::println("Equipped: {} ({} dmg)",
               weapon ? std::string_view{weapon->get().name} : "bare hands",
               weapon ? weapon->get().value : player.attack_power());

  // 2b. Full Inventory (printed every step by design)
  std::println("Inventory:");
  auto any_items = false;
  for (std::size_t i{}; i < player.inventory.items.size(); ++i) {
    if (const auto& item = player.inventory.items[i]) {
      std::println("  [{}] {} ({}, {})", i, item->name, name(item->type), item->value);
      any_items = true;
    }
  }
  if (!any_items) {
    std::println("  (empty)");
  }

  // 3. Room Line
  std::println("{}\n{}", room.name, room.description);

  // 4. Monster Line
  if (room.has_monster()) {
    std::println("A {} ({}/{}) blocks your path.", name(room.monster->type),
                 room.monster->health.current, room.monster->health.maximum);
  } else {
    std::println("The room is quiet.");
  }

  // 5. Item Line
  if (room.has_item()) {
    std::println("You see {} on the floor.", room.room_item->name);
  } else {
    std::println("There is nothing to pick up here.");
  }

  std::print("Exits:");
  for (const auto d :
       std::array{Direction::North, Direction::East, Direction::South, Direction::West}) {
    const auto exit = room.exit_at(d);
    std::print(" {}: {}", name(d), exit ? dun.rooms[*exit].name : "—");
  }
  std::println();
}

int main() {
  std::println("You are exploring the ruins of an ancient fortress.");
  std::println("Something beneath the dungeon has awakened.");
  std::println("Find the source of the disturbance and escape alive.");

  auto dungeon = make_dungeon();
  Character hero{.name = "Adventurer"};
  auto win = false;
  auto died = false;

  while (true) {
    describe(dungeon, hero);
    std::print("> ");
    std::string line;
    if (!std::getline(std::cin, line)) {
      break;
    }

    std::istringstream words{line};
    std::string word, arg;
    words >> word;

    if (word.empty()) {
      continue;
    }

    const auto command = parse_command(word);
    if (!command) {
      std::println("Huh?");
      continue;
    }

    if (*command == CommandType::Exit) {
      break;
    }

    if (const auto direction = to_direction(*command)) {
      if (!dungeon.move(*direction)) {
        std::println("There is no exit in that direction");
      } else {
        std::println("You step into {}.", dungeon.current_room().name);
      }
      continue;
    }

    if (*command == CommandType::Attack) {
      Room& room = dungeon.current_room();

      const optional<MonsterType> foe_type =
          room.has_monster() ? optional{room.monster->type} : std::nullopt;

      const auto result = combat_round(hero, room);
      if (!result) {
        std::println("There is nothing to fight here.");
        continue;
      }
      if (result->dealt == 0) {
        std::println("It collapses without a fight");
        continue;
      }

      std::println("You strike for {} damage.", result->dealt);
      if (result->foe_slain) {
        std::println("You slew the {}!", name(*foe_type));
        continue;
      }

      std::println("The {} hits back for {}.", name(*foe_type), result->taken);
    } else {
      words >> arg;  // PickUp / Use share slot syntax
      const auto slot = parse_slot(arg);

      if (*command == CommandType::PickUp) {
        if (!slot) {
          std::println("Pick up what (slot)?");
          continue;
        }
        if (*slot >= inventory_capacity) {
          std::println("No such slot.");
          continue;
        }

        Room& room = dungeon.current_room();
        if (!room.has_item()) {
          std::println("There is nothing here to pick up.");
          continue;
        }
        if (room.has_monster() && room.monster->is_not_dead()) {
          std::println("The {} won't let you take that.", name(room.monster->type));
          continue;
        }
        if (hero.inventory.get_item(*slot)) {
          std::println("Slot {} is occupied.", *slot);
          continue;
        }

        Item loot = std::move(*room.release_item());  // mutate ONLY after all four guards
        hero.pick_item(loot, *slot);
        std::println("You pick up the {}.", loot.name);
        if (loot.type == ItemType::QuestItem) win = true;
      } else {
        if (!slot) {
          std::println("Use what (slot)?");
          continue;
        }

        std::string item_name;
        if (const auto ref = hero.inventory.get_item(*slot)) item_name = ref->get().name;

        const auto used = hero.use_item(*slot);
        if (!used) {
          switch (used.error()) {
            case UseItemError::UsageError:
              std::println("You don't have a usable item in slot {}.", *slot);
              break;
            case UseItemError::AlreadyAtFullHealth:
              std::println("You are already at full health.");
              break;
            case UseItemError::CannotUseDirectly:
              std::println("You can't use that directly.");
              break;
          }
          continue;
        }
        std::println("You use the {}.", item_name);
      }
    }

    if (hero.is_dead()) {
      std::println("You have died. The dungeon keeps your bones.");
      died = true;
      break;
    }
  }

  if (!died) {
    if (win) {
      std::println("You escape with the Ancient Pendant. The dungeon is stilled. Victory!");
    } else {
      std::println("You slip out into the night, its secret unfound.");
    }
  }

  return 0;
}
