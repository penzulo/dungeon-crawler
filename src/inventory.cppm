export module dungeon.inventory;
import std;
import dungeon.item;

export using std::uint8_t, std::expected, std::unexpected, std::array, std::optional, std::size_t;

export constexpr uint8_t inventory_capacity{20};

export enum class InventoryError : uint8_t {
  InvalidSlot,   // Index is out of bounds
  EmptySlot,     // Slot is empty
  SlotOccupied,  // Slot is occupied by another entry
};

export using ItemRef = std::reference_wrapper<const Item>;

export struct Inventory {
  array<optional<Item>, inventory_capacity> items;

  auto get_item(const uint8_t index) const -> expected<ItemRef, InventoryError> {
    if (!in_bounds(index)) {
      return unexpected(InventoryError::InvalidSlot);
    }

    if (!items[index]) {
      return unexpected(InventoryError::EmptySlot);
    }

    return std::cref(items[index].value());
  }

  auto take_item(const uint8_t index) -> expected<Item, InventoryError> {
    if (!in_bounds(index)) {
      return unexpected(InventoryError::InvalidSlot);
    }

    if (!items[index]) {
      return unexpected(InventoryError::EmptySlot);
    }

    Item taken = std::move(items[index].value());
    items[index].reset();
    return taken;
  }

  auto add_item(Item new_item, const uint8_t index) -> expected<void, InventoryError> {
    if (!in_bounds(index)) {
      return unexpected(InventoryError::InvalidSlot);
    }

    if (items[index]) {
      return unexpected(InventoryError::SlotOccupied);
    }

    items[index] = std::move(new_item);
    return {};
  }

 private:
  constexpr auto in_bounds(const uint8_t index) const -> bool { return index < items.size(); }
};

export template <>
struct std::formatter<Inventory, char> {
  constexpr auto parse(std::format_parse_context& ctx) const
      -> std::format_parse_context::iterator {
    return ctx.begin();
  }

  auto format(const Inventory& inv, std::format_context& ctx) const
      -> std::format_context::iterator {
    for (size_t i{}; i < inv.items.size(); i++) {
      if (inv.items[i]) {
        ctx.advance_to(std::format_to(ctx.out(), "[{}] {} ({} - {})\n", i, inv.items[i]->name,
                                      inv.items[i]->type, inv.items[i]->value));
      }
    }
    return ctx.out();
  }
};
