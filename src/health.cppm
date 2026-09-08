export module dungeon.health;
import std;

export using std::uint8_t, std::expected, std::unexpected;

export enum HealthError : std::uint8_t {
  AlreadyDead,         // Can't damage more
  AlreadyFullyHealed,  // Can't heal more
};

/**
 * This is the health struct. Individually it has no meaning.
 * If something HAS a health (e.g. Player, Monster), then we
 * can include the health struct with that.
 *
 * We track current health (may be zero) and we also track the
 * maximum allowed health.
 */
export struct Health {
  uint8_t current{100};
  uint8_t maximum{100};

  Health() = default;
  explicit Health(const uint8_t new_current) : current(new_current) {}
  Health(const uint8_t nc, const uint8_t n_max) : current(nc), maximum(n_max) {}

  /**
   * Calculates the damage taken to the health.
   * Reduces the health by the calculated amount.
   * Returns the amount of damage took.
   */
  expected<uint8_t, HealthError> take_damage(const uint8_t damage) noexcept {
    if (current == 0) {
      return unexpected(HealthError::AlreadyDead);
    }

    const uint8_t damage_took = (damage >= current) ? current : damage;
    current -= damage_took;
    return damage_took;
  }

  /**
   * Calculates the healing taken to the health.
   * Increases the health by the calculated amount.
   * Returns the amount of healing taken.
   */
  expected<uint8_t, HealthError> take_heal(const uint8_t increase) noexcept {
    if (current == maximum) {
      return unexpected(HealthError::AlreadyFullyHealed);
    }
    const uint8_t can_heal = maximum - current;
    const uint8_t heal_taken = (increase <= can_heal) ? increase : can_heal;
    current += heal_taken;
    return heal_taken;
  }
};
