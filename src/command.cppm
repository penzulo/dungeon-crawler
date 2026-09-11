export module dungeon.command;
import std;
import dungeon.room;

using std::optional, std::uint8_t, std::string_view, std::tolower;

export enum class CommandType : uint8_t {
  North,   // Go North
  East,    // Go East
  South,   // Go South
  West,    // Go West
  Attack,  // Attack with or without weapon
  PickUp,  // Pick up nearby item
  Use,     // Use Item
  Exit     // Exit the game
};

export auto to_direction(const CommandType command) noexcept -> optional<Direction> {
  switch (command) {
    case CommandType::North: return Direction::North;
    case CommandType::East: return Direction::East;
    case CommandType::South: return Direction::South;
    case CommandType::West: return Direction::West;

    case CommandType::Attack:
    case CommandType::PickUp:
    case CommandType::Use:
    case CommandType::Exit: return std::nullopt;
  }
  std::unreachable();
}

export auto parse_command(string_view word) -> optional<CommandType> {
  if (word.empty()) {
    return std::nullopt;
  }

  switch (std::tolower(word.front())) {
    case 'n': return CommandType::North;
    case 'e': return CommandType::East;
    case 's': return CommandType::South;
    case 'w': return CommandType::West;
    case 'a': return CommandType::Attack;
    case 'p': return CommandType::PickUp;
    case 'u': return CommandType::Use;
    case 'x': return CommandType::Exit;
    default: return std::nullopt;
  }
}

export auto parse_slot(string_view token) noexcept -> optional<uint8_t> {
  int value{};
  const auto [ptr, ec] = std::from_chars(token.begin(), token.end(), value);

  if (ec != std::errc{}) {
    return std::nullopt;
  }

  if (ptr != token.end()) {
    return std::nullopt;
  }

  if (value < 0 || value > 255) {
    return std::nullopt;
  }
  return static_cast<uint8_t>(value);
}
