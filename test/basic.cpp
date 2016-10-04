#define CATCH_CONFIG_MAIN
#include <catch.hpp>
#include <string>
#include <tuple>

#include <elix/elix.hpp>

using namespace elix::literals;

struct Position
{
    float x;
    float y;

    static constexpr auto component_def =
        elix::component_def<Position, float, float>("position",
        {"x", &Position::x},
        {"y", &Position::y});
};

// I like C++ sooo much
constexpr decltype(Position::component_def) Position::component_def;

struct Spell
{
    std::string name;
    int damage;

    static constexpr auto component_def =
        elix::component_def<Spell, std::string, int>("spell",
        {"name", &Spell::name},
        {"damage", &Spell::damage});
};

constexpr decltype(Spell::component_def) Spell::component_def;

TEST_CASE( "Basic test", "[Basic]")
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
                "damage": 42
            }
        },
        "Wizard2": {
            "position": {
                "x": 11.2,
                "y": 7.6
            },
            "spell": {
                "name": "Spell2",
                "damage": 13
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

    auto wizards = elix::load<Position, Spell>(wizards_json);
    for(auto wizard : wizards) {
        std::cout << wizard.name << '\n';
        auto pos = wizard.get<Position>();
        if (pos) {
            std::cout << "Position: " << pos->x << ' ' << pos->y << '\n';
        }
        auto spell = wizard.get<Spell>();
        if (spell) {
            std::cout << "Spell: " << spell->name << ' ' << spell->damage << '\n';
        } else {
            std::cout << wizard.name << " is not a wizard\n";
        }
    }
}
