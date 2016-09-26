#define CATCH_CONFIG_MAIN
#include <catch.hpp>
#include <string>
#include <array>
#include <tuple>
#include <functional>
#include <utility>
#include <experimental/any>

#include <ecf/ecf.hpp>

using vec3 = std::array<float, 3>;

class transform_component{
public:
    transform_component() = default;
    transform_component(std::tuple<vec3, std::string> prop_vals):
        position_(std::get<0>(prop_vals)),
        tag_(std::get<1>(prop_vals))
    {}

    vec3 &position() { return position_; }
    std::string &tag() { return tag_; }

    static constexpr auto component_def =
        ecf::component_def<vec3, std::string>("transform", {
            "position",
            "tag"
            });

private:
    vec3 position_;
    std::string tag_;
};

constexpr decltype(transform_component::component_def)
    transform_component::component_def;

class render_component {
public:
    using property_types = std::tuple<transform_component>;

    render_component() = default;
    render_component(property_types &&prop_vals):
        transform_(std::get<0>(prop_vals))
    {}

    static constexpr auto component_def =
        ecf::component_def<transform_component>("render", {
            "transform"
            });


private:
    transform_component transform_;
};

constexpr decltype(render_component::component_def)
    render_component::component_def;

TEST_CASE( "Basic test", "[Basic]")
{
    auto json = R"(
    [
        {
            "__name": "Entity1",
            "transform": {
                "position": [1, 2, 3],
                "tag": "value"
            },
            "render": {
                "transform": 1
            }
        },
        {
            "__name": "Entity2",
            "transform": {
                "position": [1, 2, 3],
                "tag": "value"
            },
            "render": {
                "transform": 1
            }
        }
    ]
    )";

    auto entities = ecf::load<transform_component, render_component>(json);
    for (auto &e : entities) {
        std::cout << e.name << '\n';
        auto transform = e.get<transform_component>();
        REQUIRE (transform != nullptr);
        REQUIRE (transform->position()[0] == 0);
    }
}
