export module dungeon.monster;
import dungeon.health;

export enum MonsterType : unsigned char { Goblin, Skeleton, Wolf, Troll };

export struct Monster {
  MonsterType type;
  Health health;
};