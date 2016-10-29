#define CATCH_CONFIG_MAIN
#include <catch.hpp>
#include <string>
#include <tuple>

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

TEST_CASE( "Decode test", "[Decode]")
{
    auto wizards_json = R"(
    {
        "Wizard1": {
            "position": {
                "x": 10.5,
                "y": 19.3
            },
            "spell": {
                "name": "Spell1",
                "damage": 42,
                "position": {
                    "x": 15,
                    "y": 13
                }
            }
        },
        "Wizard2": {
            "position": {
                "x": 11.2,
                "y": 7.6
            },
            "spell": {
                "name": "Spell2",
                "damage": 13,
                "position": {
                    "x": 12,
                    "y": 16.8
                }
            }
        },
        "Wizard3": {
            "position": {
                "x": 3.3,
                "y": 0
            }
        }
    }
    )";

    auto wizards = elix::decode<Position, Spell>(wizards_json);
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
            std::cout << wizard.name << " is not a wizard\n";
        }
    }
}

TEST_CASE("Encode test" "[Encode]")
{
    elix::entity<Position, Spell> wizard("wizard");
    auto &pos = wizard.get<Position>();
    pos = new Position();
    pos->x = 10;
    pos->y = 20;
    auto &spell = wizard.get<Spell>();
    spell = new Spell();
    spell->name = "spell";
    spell->position = *pos;
    spell->damage = 13;

    std::cout << elix::encode<Position, Spell>({wizard}).dump(4) << '\n';
}
