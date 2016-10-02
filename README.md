# elix

elix is a C++ entity-component (de)serialization library without code generation.
This literally means that C++ compiler generates all the code required for (de)serialization with as little help from user as possible.

## Dependencies

Elix relies on the [json](https://github.com/nlohmann/json) library and currently supports only deserialization from JSON,
but this is going to change soon.

## Example

```c++
#include <elix/elix.hpp>
#include <iostream>

// The component structure
struct Position
{
    using property_types = std::tuple<float, float>;
    // This is actually how the component will be constructed by elix
    Position(property_types &&prop_vals):
        x(std::get<0>(prop_vals)),
        y(std::get<1>(prop_vals))
    {}

    // Define component name, property names and property types
    static constexpr auto component_def =
        elix::component_def<float, float>("position", {
            "x", // property "x" of type float
            "y"  // property "y" of type float
        });

    float x;
    float y;
};

// I like C++ sooo much
constexpr decltype(Position::component_def) Position::component_def;

// Another component
struct Spell
{
    using property_types = std::tuple<std::string, int>;
    Spell(property_types &&prop_vals):
        name(std::get<0>(prop_vals)),
        damage(std::get<1>(prop_vals))
    {}

    static constexpr auto component_def =
        elix::component_def<std::string, int>("spell", {
            "name",
            "damage"
        });

    std::string name;
    int damage;
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
            "damage": 42
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

// Load JSON into a list of entities.
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
```

## Current limitations

The project is under early development and currently has more limitations than features:

* Supports JSON only.
* No serialization support at the moment.
* Supported component's property types are limited to those supported by [json](https://github.com/nlohmann/json) library.
* Nested components are not supported (this feature is under active development).
* Unfriendly compile-time errors reporting.
* No support for custom component allocators.
