export module dungeon.character;
import std;
import dungeon.health;
import dungeon.item;
import dungeon.inventory;
import dungeon.equipment;
import dungeon.monster;

export using std::expected, std::unexpected, std::uint8_t, std::string;

/*
 * Bare-Hands attack of the Character.
 */
export constexpr uint8_t base_attack{2};

/*
 * Categorizes the different errors one might encounter
 * while using items in an inventory.
 */
export enum UseItemError {
  UsageError,           // If player tries wrong access to inventory
  AlreadyAtFullHealth,  // If player tried to drink potion at full health
  CannotUseDirectly,    // If player tries to use quest item directly
};

export struct Character {
  string name;

  Health health{};
  Inventory inventory{};
  Equipment equipment{};

  auto use_item(const uint8_t slot) -> expected<void, UseItemError> {
    const auto item_ref = inventory.get_item(slot);

    if (!item_ref) {
      return unexpected(UseItemError::UsageError);
    }

    const Item& item = item_ref->get();

    switch (item.type) {
      case ItemType::Consumable: {
        if (!health.take_heal(item.value)) {
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
        return unexpected(UseItemError::CannotUseDirectly);
      }
    }
  }

  auto pick_item(const Item& visible, const uint8_t new_index) {
    return inventory.add_item(visible, new_index);
  }

  auto attack_power() const noexcept -> uint8_t {
    if (!equipment.equipped_weapon) {
      return base_attack;
    }

    const auto item = inventory.get_item(equipment.equipped_weapon.value());
    if (!item || item->get().type != ItemType::Weapon) {
      return base_attack;
    }
    return item->get().value;
  }

  auto attack(Monster& mon) noexcept -> expected<uint8_t, HealthError> {
    return mon.health.take_damage(attack_power());
  }

  auto is_dead() const noexcept -> bool { return health.is_dead(); }
};
