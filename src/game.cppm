module;
#include <expected>
#include <utility>
export module dungeon.game;
import std;
export import dungeon.dungeon;
export import dungeon.character;
export import dungeon.command;
import dungeon.monster;
import dungeon.inventory;
import dungeon.item;
import dungeon.room;

using std::uint8_t, std::string, std::expected, std::optional, std::format, std::println;

export enum GameState : uint8_t {
  Playing,  // User is playing
  Dead,     // User's character is dead
  Won,      // User's character finished the game
  Quit      // User chose to quit
};

export enum GameError : uint8_t {
  NotAnExit,            // move into a direction with no exit
  NothingToFight,       // attack with no (living) monster present
  NoItemOnFloor,        // pickup in a room with no item
  GuardedByMonster,     // pickup while a live monster is present
  NoSuchSlot,           // PickUp/Use and slot >= inventory_capacity
  EmptySlot,            // Use and the slot is empty / not an item
  SlotOccupied,         // PickUp into an already-occupied slot
  AlreadyAtFullHealth,  // Use a consumable at full HP
  CannotUseDirectly,    // Use a quest item
};

export struct TurnResult {
  GameState state;
  string narration;
};

export struct UserIntent {
  CommandType command;
  optional<uint8_t> slot;
};

export struct Game {
  Dungeon dungeon;
  Character hero;
  GameState state{GameState::Playing};

  auto next_state(const UserIntent intent) -> expected<TurnResult, GameError> {
    const auto result = [&]() -> expected<TurnResult, GameError> {
      if (intent.command == CommandType::Exit) {
        return TurnResult{GameState::Quit, ""};
      }

      if (const auto direction = to_direction(intent.command)) {
        return dungeon.move(*direction)
            .transform_error([](const auto) { return GameError::NotAnExit; })
            .transform([&]() {
              return TurnResult{GameState::Playing,
                                format("You step into {}.", dungeon.current_room().name)};
            });
      }

      if (intent.command == CommandType::Attack) {
        auto& room = dungeon.current_room();
        const auto foe_type = room.has_monster() ? optional(room.monster->type) : std::nullopt;

        return combat_round(hero, room)
            .transform_error([](const auto) { return GameError::NothingToFight; })
            .transform([&](const CombatResult& result) {
              const auto narration =
                  format("You strike for {} damage.", result.dealt) +
                  (result.foe_slain
                       ? format("You slew the {}!", name(*foe_type))
                       : format("The {} hits back for {}.", name(*foe_type), result.taken)) +
                  (hero.is_dead() ? "You have died. The dungeon keeps your bones." : "");
              return TurnResult{hero.is_dead() ? GameState::Dead : GameState::Playing, narration};
            });
      }

      if (intent.command == CommandType::PickUp) {
        auto& room = dungeon.current_room();

        const auto guarded = [&]() -> expected<void, GameError> {
          if (!intent.slot || *intent.slot >= inventory_capacity) {
            return std::unexpected(GameError::NoSuchSlot);
          }
          if (!room.has_item()) {
            return std::unexpected(GameError::NoItemOnFloor);
          }
          if (room.has_monster() && room.monster->is_not_dead()) {
            return std::unexpected(GameError::GuardedByMonster);
          }
          if (hero.inventory.get_item(*intent.slot)) {
            return std::unexpected(GameError::SlotOccupied);
          }
          return {};
        }();

        return guarded.and_then([&]() -> expected<TurnResult, GameError> {
          const Item loot = std::move(*room.release_item());
          hero.pick_item(loot, *intent.slot);
          const auto won = loot.type == ItemType::QuestItem;
          return TurnResult{won ? GameState::Won : GameState::Playing,
                            format("You pick up the {}.", loot.name)};
        });
      }

      if (intent.command == CommandType::Use) {
        if (!intent.slot || *intent.slot >= inventory_capacity) {
          return unexpected(GameError::NoSuchSlot);
        }
        const auto ref = hero.inventory.get_item(*intent.slot);
        if (!ref) {
          return unexpected(GameError::EmptySlot);
        }
        const string name = ref->get().name;

        return hero.use_item(*intent.slot)
            .transform_error([](const UseItemError e) {
              switch (e) {
                case UseItemError::UsageError: return GameError::EmptySlot;
                case UseItemError::CannotUseDirectly: return GameError::CannotUseDirectly;
                case UseItemError::AlreadyAtFullHealth: return GameError::AlreadyAtFullHealth;
              }
              std::unreachable();
            })
            .transform(
                [&] { return TurnResult{GameState::Playing, format("You use the {}.", name)}; });
      };
      std::unreachable();
    }();

    if (result) {
      state = result->state;
    }
    return result;
  }
};

export auto describe(const Dungeon& dun, const Character& player) -> void {
  const auto& room = dun.current_room();
  const auto weapon = player.equipment.equipped_weapon.and_then([&player](const uint8_t index) {
    const auto res = player.inventory.get_item(index);
    return res ? optional{res.value()} : std::nullopt;
  });

  println("{} — HP {}/{}", player.name, player.health.current, player.health.maximum);
  println("Equipped: {} ({} dmg)", weapon ? std::string_view{weapon->get().name} : "bare hands",
          weapon ? weapon->get().value : player.attack_power());
  println("Inventory:\n{}", player.inventory);
  println("{}\n{}", room.name, room.description);

  if (room.has_monster()) {
    println("A {} ({}/{}) blocks your path.", name(room.monster->type),
            room.monster->health.current, room.monster->health.maximum);
  } else {
    println("The room is quiet.");
  }

  if (room.has_item()) {
    println("You see {} on the floor.", room.room_item->name);
  } else {
    println("There is nothing to pick up here.");
  }

  std::print("Exits:");
  for (const auto d :
       std::array{Direction::North, Direction::East, Direction::South, Direction::West}) {
    const auto exit = room.exit_at(d);
    std::print(" {}: {}", name(d), exit ? dun.rooms[*exit].name : "—");
  }
  println();
}

export auto error_message(const GameError error, const Game& game, const CommandType,
                          const optional<uint8_t> slot) -> string {
  switch (error) {
    case GameError::NotAnExit: return "There is no exit in that direction";
    case GameError::NothingToFight: return "There is nothing to fight here.";
    case GameError::NoItemOnFloor: return "There is nothing here to pick up.";
    case GameError::GuardedByMonster:
      return format("The {} won't let you take that.",
                    name(game.dungeon.current_room().monster->type));
    case GameError::NoSuchSlot: return "No such slot.";
    case GameError::EmptySlot:
      return format("You don't have a usable item in slot {}.", slot.value_or(0));
    case GameError::SlotOccupied: return format("Slot {} is occupied.", slot.value_or(0));
    case GameError::AlreadyAtFullHealth: return "You are already at full health.";
    case GameError::CannotUseDirectly: return "You can't use that directly.";
  }
  std::unreachable();
}
