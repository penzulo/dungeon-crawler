export module dungeon.character;
import std;
import dungeon.health;
import dungeon.item;
import dungeon.inventory;
import dungeon.equipment;

export using std::expected, std::unexpected, std::uint8_t;

/*
 * Categorizes the different errors a character can encounter
 * at the character abstraction level.
 */
export enum UseItemError {
  UsageError,           // If player tries wrong access to inventory
  AlreadyAtFullHealth,  // If player tried to drink potion at full health
  CannnotUseDirectly,   // If player tries to use quest item directly
};

export struct Character {
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

  expected<void, InventoryError> pick_item(const Item& visible,
                                           const uint8_t new_index) {
    return inventory.add_item(visible, new_index);
  }
};
