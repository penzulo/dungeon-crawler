export module dungeon.monster;
import std;
import dungeon.health;

export using std::uint8_t, std::string_view;

export enum MonsterType : uint8_t { Goblin, Skeleton, Wolf, Troll };

export struct MonsterStats {
  uint8_t damage;
  uint8_t max_health;
};

export constexpr auto stats(const MonsterType type) noexcept -> MonsterStats {
  switch (type) {
    case MonsterType::Goblin: return MonsterStats{3, 20};
    case MonsterType::Skeleton: return MonsterStats{4, 35};
    case MonsterType::Wolf: return MonsterStats{5, 25};
    case MonsterType::Troll: return MonsterStats{8, 50};
  }
  std::unreachable();
}

export auto name(const MonsterType type) noexcept -> string_view {
  switch (type) {
    case MonsterType::Goblin: return "Goblin";
    case MonsterType::Skeleton: return "Skeleton";
    case MonsterType::Wolf: return "Wolf";
    case MonsterType::Troll: return "Troll";
  }
  std::unreachable();
}

export struct Monster {
  MonsterType type{MonsterType::Goblin};
  Health health;

  Monster() = default;
  explicit Monster(const MonsterType n_type)
      : type(n_type), health{stats(n_type).max_health, stats(n_type).max_health} {}

  auto is_dead() const noexcept -> bool { return health.is_dead(); }
  auto is_not_dead() const noexcept -> bool { return health.is_not_dead(); }
};
