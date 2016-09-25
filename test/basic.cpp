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

class transform_component {
public:
    transform_component() = default;
    transform_component(const vec3& position,
            const std::string &tag):
        position_(position),
        tag_(tag)
    {}

    vec3 &position() { return position_; }
    std::string &tag() { return tag_; }

    static const char *properties[];
    static constexpr auto name = "transform";

private:
    vec3 position_;
    std::string tag_;
};

const char * transform_component::properties[] = {
        "position",
        "tag"
    };

class render_component {
public:
    render_component() = default;
    render_component(const transform_component &transform):
        transform_(transform)
    {}

    static const char *properties[];
    static constexpr auto name = "render";

private:
    transform_component transform_;
};

const char *render_component::properties[] = {
    "transform"
};

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
