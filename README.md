# elix

elix is a C++ entity-component (de)serialization library without code generation.
This literally means that C++ compiler generates all the code required for (de)serialization with as little help from user as possible.

## Dependencies

Elix relies on the [json](https://github.com/nlohmann/json) library and currently supports only deserialization from JSON,
but this is going to change soon.

## Example

```c++
// The component structure
struct Position
{
    float x;
    float y;

    // Specify cpomponent's name, property types and property names
    static constexpr auto component_def =
        elix::component_def<Position, float, float>("position",
        {"x", &Position::x},
        {"y", &Position::y});
};

// I like C++ sooo much
constexpr decltype(Position::component_def) Position::component_def;

// Another component
struct Spell
{
    std::string name;
    int damage;
    // You can nest components
    Position position;

    static constexpr auto component_def =
        elix::component_def<Spell, std::string, int, Position>("spell",
        {"name", &Spell::name},
        {"damage", &Spell::damage},
        {"position", &Spell::position});
};

constexpr decltype(Spell::component_def) Spell::component_def;

//...

// This is the input JSON
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
                "x": 8.8,
                "y": 0.1
            }
        }
    },
    "Wizard2": {
        "position": {
            "x": 3.3,
            "y": 0
        }
    }
}
)";

// Load JSON into list of entities
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
        std::cout << "Spell position: " << spell->position.x << ' ' << spell->position.y << '\n';
    } else {
        std::cout << wizard.name << " is not a wizard\n";
    }
}
```

## Current limitations

The project is under early development and currently has more limitations than features:

* Supports JSON only.
* No serialization support at the moment.
* Supported component's property types are limited to those supported by [json](https://github.com/nlohmann/json) library.
* ~~Nested components are not supported~~.
* Unfriendly compile-time errors reporting.
* No support for custom component allocators.
