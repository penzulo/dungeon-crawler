#include <expected>
#include <print>

using std::expected, std::unexpected, std::optional, std::string, std::array;

constexpr uint8_t inventory_capacity{20};
constexpr uint8_t n_rooms{5};

enum HealthError : unsigned char {
  AlreadyDead,         // Can't damage more
  AlreadyFullyHealed,  // Can't heal more
};

/**
 * This is the health struct. Individually it has no meaning.
 * If something HAS a health (e.g. Player, Monster), then we
 * can include the health struct with that.
 *
 * We track current health (may be zero) and we also track the
 * maximum allowed health.
 */
struct Health {
  uint8_t current{100};
  uint8_t maximum{100};

  Health() = default;
  explicit Health(const uint8_t new_current) : current(new_current) {}
  Health(const uint8_t nc, const uint8_t n_max) : current(nc), maximum(n_max) {}

  /**
   * Calculates the damage taken to the health.
   * Reduces the health by the calculated amount.
   * Returns the amount of damage took.
   */
  expected<uint8_t, HealthError> take_damage(const uint8_t damage) noexcept {
    if (current == 0) {
      return unexpected(HealthError::AlreadyDead);
    }

    const uint8_t damage_took = (damage >= current) ? current : damage;
    current -= damage_took;
    return damage_took;
  }

  /**
   * Calculates the healing taken to the health.
   * Increases the health by the calculated amount.
   * Returns the amount of healing taken.
   */
  expected<uint8_t, HealthError> take_heal(const uint8_t increase) noexcept {
    if (current == maximum) {
      return unexpected(HealthError::AlreadyFullyHealed);
    }
    const uint8_t can_heal = maximum - current;
    const uint8_t heal_taken = (increase <= can_heal) ? increase : can_heal;
    current += heal_taken;
    return heal_taken;
  }
};

/**
 * Abstract Item type. Can be found inside Rooms or
 * can be held by the inventory of a character.
 */
// using Item = std::variant<Weapon, Consumable, QuestItem>;
enum ItemType { Weapon, Consumable, QuestItem };

struct Item {
  ItemType type{ItemType::Consumable};
  string name;

  uint8_t value{};

  Item() = default;
  explicit Item(const string& item_name) : name(item_name) {}
  Item(const string& item_name, const ItemType& type, const uint8_t& val)
      : name(item_name), type(type), value(val) {}
};

enum InventoryError {
  InvalidSlot,   // Index is out of bounds
  EmptySlot,     // Slot is empty
  SlotOccupied,  // Slot is occupied by another entry
};

struct Inventory {
  array<optional<Item>, inventory_capacity> items;

  Inventory() = default;

  expected<Item, InventoryError> get_item(const uint8_t index) const {
    if (index >= items.size()) {
      return unexpected(InventoryError::InvalidSlot);
    }

    if (!items[index].has_value()) {
      return unexpected(InventoryError::EmptySlot);
    }

    return items[index].value();
  }

  expected<Item, InventoryError> take_item(const uint8_t index) {
    if (index >= items.size()) {
      return unexpected(InventoryError::InvalidSlot);
    }

    if (!items[index].has_value()) {
      return unexpected(InventoryError::EmptySlot);
    }

    Item taken = std::move(items[index].value());
    items[index].reset();
    return taken;
  }

  expected<void, InventoryError> add_item(const Item& new_item,
                                          const uint8_t index) {
    if (index >= items.size()) {
      return unexpected(InventoryError::InvalidSlot);
    }

    if (items[index].has_value()) {
      return unexpected(InventoryError::EmptySlot);
    }

    items[index] = new_item;

    return {};
  }
};

// Items which are currently being used.
struct Equipment {
  std::optional<size_t> equipped_weapon;

  Equipment() = default;
};

enum UseItemError {
  UsageError,           // If player tries wrong access to inventory
  AlreadyAtFullHealth,  // If player tried to drink potion at full health
  CannnotUseDirectly,   // If player tries to use quest item directly
};

struct Character {
  int id{};
  std::string name;
  Health health;
  Inventory inventory;
  Equipment equipment;

  Character() = default;

  expected<void, UseItemError> use_item(const uint8_t& slot) {
    const auto item = inventory.get_item(slot);

    if (!item.has_value()) {
      return unexpected(UseItemError::UsageError);
    }

    switch (item.value().type) {
      case ItemType::Consumable: {
        const auto res = health.take_heal(item.value().value);
        if (!res.has_value()) {
          return unexpected(UseItemError::AlreadyAtFullHealth);
        }
        inventory.take_item(slot);
        return {};
      }

      case ItemType::Weapon: {
        equipment.equipped_weapon = slot;
        return {};
      }

      case ItemType::QuestItem: {
        return unexpected(UseItemError::CannnotUseDirectly);
      }
    }
  }

  void pick_item(const Item& visible, const uint8_t new_index) {
    const auto result = inventory.add_item(visible, new_index);
    if (!result.has_value()) {
      // TODO: Return an error here. But what kind?
      return;
    }
  }
};

enum MonsterType : unsigned char { Goblin, Skeleton, Wolf, Troll };

struct Monster {
  MonsterType type;
  Health health;
};

// Decide exact functionality later
struct Room {
  std::string name;
  std::string description;

  Monster danger;  // can there be a better name for this?
  Item room_item;
};

struct Dungeon {
  std::string name;
  std::array<Room, n_rooms> rooms;
};

enum CommandType : unsigned char {
  Enter,      // Enter room or dungeon
  Forward,    // Go forward
  Backward,   // Go backward
  Left,       // Go left
  Right,      // Go right
  PickItem,   // Add item to inventory
  EquipItem,  // Equip item/weapon
  UseItem,    // Use item (e.g potion)
  Attack,     // Attack Monster
  Flee,       // Flee from action
  Exit,       // Exit game/dungeon
};

int main() {
  std::println("You are exploring the ruins of an ancient fortress.");
  std::println("Something beneath the dungeon has awakened.");
  std::println("Find the source of the disturbance and escape alive.");

  return 0;
}
