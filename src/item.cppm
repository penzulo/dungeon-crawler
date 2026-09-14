export module dungeon.item;
import std;

export using std::string, std::uint8_t, std::string_view;

/**
 * Abstract Item type. Can be found inside Rooms or
 * can be held by the inventory of a character.
 */
export enum ItemType { Weapon, Consumable, QuestItem };

export template <>
struct std::formatter<ItemType> : std::formatter<string_view> {
  auto format(const ItemType it, std::format_context& ctx) const {
    const string_view name = [&it]() {
      switch (it) {
        case ItemType::Weapon: return "Weapon";
        case ItemType::QuestItem: return "Quest Item";
        case ItemType::Consumable: return "Consumable";
      }
      std::unreachable();
    }();
    return std::formatter<string_view>::format(name, ctx);
  }
};

export auto name(const ItemType type) noexcept -> string_view {
  switch (type) {
    case ItemType::Weapon: return "Weapon";
    case ItemType::Consumable: return "Consumable";
    case ItemType::QuestItem: return "QuestItem";
  }
  std::unreachable();
}

export struct Item {
  ItemType type{ItemType::Consumable};
  string name;

  uint8_t value{};

  Item() = default;
  explicit Item(const string& item_name) : name(item_name) {}
  Item(const string& item_name, const ItemType& type, const uint8_t& val)
      : type(type), name(item_name), value(val) {}
};
