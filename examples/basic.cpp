#include <string>
#include <iostream>
#include <fstream>
#include <cstdlib>

#include <elix/elix.hpp>

struct Position
{
    double x;
    double y;

    static constexpr auto component_def =
        elix::component_def<Position, double, double>("position",
        {"x", &Position::x},
        {"y", &Position::y});
};

// I like C++ sooo much
constexpr decltype(Position::component_def) Position::component_def;

struct Spell
{
    std::string name;
    int damage;
    Position position;

    static constexpr auto component_def =
        elix::component_def<Spell, std::string, int, Position>("spell",
        {"name", &Spell::name},
        {"damage", &Spell::damage},
        {"position", &Spell::position});
};

constexpr decltype(Spell::component_def) Spell::component_def;


int main(int argc, char *argv[]) {

    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " json_file" << std::endl;
        return EXIT_FAILURE;
    }

    std::ifstream infile(argv[1]);
    if (!infile.is_open()) {
        std::cerr << "Failed to open " << argv[2] << '\n';
        return EXIT_FAILURE;
    }

    std::string wizards_json({std::istreambuf_iterator<char>(infile), std::istreambuf_iterator<char>()});

    auto wizards = elix::decode<Position, Spell>(wizards_json);

    std::cout << "Deserialized: \n";
    for(auto wizard : wizards) {
        std::cout << wizard.name << '\n';

        auto pos = wizard.get<Position>();
        if (pos) {
            std::cout << "Position: " << pos->x << ' ' << pos->y << '\n';
        }

        auto spell = wizard.get<Spell>();
        if (spell) {
            std::cout << "Spell: " << spell->name << ' ' << spell->damage << '\n';
            std::cout << "Spell position: " << spell->position.x << ' ' << spell->position.y << '\n';
        } else {
            std::cout << wizard.name << " is not a wizard" << std::endl;
        }
    }

    std::cout << "Serialized: \n";
    std::cout << elix::encode<Position, Spell>(wizards).dump(4) << '\n';

    return EXIT_SUCCESS;
}
