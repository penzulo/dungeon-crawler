export module dungeon.dungeon;
import std;
import dungeon.room;

export using std::uint8_t;

export constexpr uint8_t n_rooms{5};

export struct Dungeon {
  std::string name;
  std::array<Room, n_rooms> rooms;
};