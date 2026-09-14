import std;
import dungeon.game;

using std::optional, std::string, std::expected, std::unexpected, std::println, std::string_view;

auto parse_turn(std::istringstream& words, const string_view word) -> expected<UserIntent, string> {
  const auto command = parse_command(word);
  if (!command) {
    return unexpected("Huh?");
  }

  optional<uint8_t> slot;
  if (*command == CommandType::PickUp || *command == CommandType::Use) {
    std::string arg;
    words >> arg;
    slot = parse_slot(arg);
    if (!slot) {
      return unexpected(*command == CommandType::PickUp ? "Pick up what (slot)?"
                                                        : "Use what (slot)?");
    }
  }
  return UserIntent{*command, slot};
}

int main() {
  println("You are exploring the ruins of an ancient fortress.");
  println("Something beneath the dungeon has awakened.");
  println("Find the source of the disturbance and escape alive.");

  Game game{.dungeon = make_dungeon(), .hero = Character{.name = "Adventurer"}};

  while (game.state == GameState::Playing) {
    describe(game.dungeon, game.hero);
    std::print("> ");
    std::string line;

    if (!std::getline(std::cin, line)) {
      break;
    }

    std::istringstream words{line};
    string word;
    words >> word;
    if (word.empty()) {
      continue;
    }

    if (const auto turn = parse_turn(words, word)) {
      if (const auto result = game.next_state(*turn)) {
        if (!result->narration.empty()) {
          println("{}", result->narration);
        }
      } else {
        println("{}", error_message(result.error(), game, turn->command, turn->slot));
      }
    } else {
      println("{}", turn.error());
    }
  }

  if (game.state == GameState::Won) {
    println("You escape with the Ancient Pendant. The dungeon is stilled. Victory!");
  } else if (game.state != GameState::Dead) {
    println("You slip out into the night, its secret unfound.");
  }

  return 0;
}
