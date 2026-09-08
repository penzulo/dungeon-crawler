export module dungeon.room;
import std;
import dungeon.monster;
import dungeon.item;

// Decide exact functionality later
export struct Room {
  std::string name;
  std::string description;

  Monster monster;
  Item room_item;
};