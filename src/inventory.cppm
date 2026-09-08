export module dungeon.inventory;
import std;
import dungeon.item;

export using std::uint8_t, std::expected, std::unexpected, std::array,
    std::optional;

export constexpr uint8_t inventory_capacity{20};

export enum InventoryError {
  InvalidSlot,   // Index is out of bounds
  EmptySlot,     // Slot is empty
  SlotOccupied,  // Slot is occupied by another entry
};

export struct Inventory {
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
      return unexpected(InventoryError::SlotOccupied);
    }

    items[index] = new_item;

    return {};
  }
};