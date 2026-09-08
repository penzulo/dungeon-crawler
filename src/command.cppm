export module dungeon.command;

export enum CommandType : unsigned char {
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