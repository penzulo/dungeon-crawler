export module dungeon.item;
import std;

export using std::string, std::uint8_t;

/**
 * Abstract Item type. Can be found inside Rooms or
 * can be held by the inventory of a character.
 */
// using Item = std::variant<Weapon, Consumable, QuestItem>;
export enum ItemType { Weapon, Consumable, QuestItem };

export struct Item {
  ItemType type{ItemType::Consumable};
  string name;

  uint8_t value{};

  Item() = default;
  explicit Item(const string& item_name) : name(item_name) {}
  Item(const string& item_name, const ItemType& type, const uint8_t& val)
      : type(type), name(item_name), value(val) {}
};