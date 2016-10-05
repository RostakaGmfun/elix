#ifndef ELIX_HPP
#define ELIX_HPP

#include <stdexcept>
#include <tuple>
#include <string>
#include <map>
#include <array>
#include <functional>
#include <type_traits>
#include <utility>

#include <json.hpp>

namespace elix {

using json = nlohmann::json;

struct component_prop_name
{
    constexpr component_prop_name(const char *name, bool is_component = false):
        name(name),
        is_component(is_component)
    {}

    const char *name;
    bool is_component;
};

namespace literals {

    constexpr component_prop_name operator""_comp(const char *name, std::size_t length)
    {
        component_prop_name component_name(name, true);
        return component_name;
    }

} // literals

template <class Component, class ... Properties>
struct component_def
{
    template <class Property>
    using property_t = std::pair<component_prop_name, Property Component::*>;

    using property_types = std::tuple<Properties...>;
    using property_names_t = std::array<component_prop_name, sizeof...(Properties)>;

    const char *name;
    std::tuple<Properties Component::*...> properties;
    property_names_t property_names;

    constexpr component_def(const char *name, property_t<Properties>&& ... properties):
        name(name),
        properties(std::make_tuple(properties.second...)),
        property_names({properties.first...})
    {}

    auto construct(const json &props) const
    {
        // check if all properties are defined
        for (auto prop : property_names) {
            auto p = props.find(prop.name);
            if (p == props.end()) {
                // Ooops, bad thing
                // There is currently no way to define default property value
                throw std::runtime_error("Property " + std::string(prop.name) +
                        " is undefined in component " + std::string(name));
            }
        }
        return construct(props, std::index_sequence_for<Properties...>{});
    }

private:
    template <std::size_t ... Is>
    auto construct(const json &props, std::index_sequence<Is...>) const
    {
        auto c = new Component;
        (void)std::initializer_list<int>{
            (c->*(std::get<Is>(properties)) = props[std::get<Is>(property_names).name], 0)...};
        return c;
    }
};

template <class ... Components>
struct entity
{
    std::tuple<Components*...> components;
    std::string name;

    template <class Component>
    Component* &get()
    {
        return std::get<Component*>(components);
    }
};

template <class ... Components>
auto make_entity(const std::string &name)
{
    entity<Components...> e;
    e.name = name;
    return e;
}

template <class ... Components>
using entity_list = std::vector<entity<Components...>>;

template <class Func, class ... C>
void for_each_type(Func func)
{
    (void)std::initializer_list<int> {(func(static_cast<C*>(nullptr)), 0)...};
}

template <class ... Components>
auto load(const std::string &jsonStr)
{
    auto&& entity_lib = json::parse(jsonStr);
    entity_list<Components...> entities;
    auto&& entity_map = entity_lib.get<std::map<std::string, json>>();

    for (const auto &entity: entity_map) {
        auto e = make_entity<Components ...>(entity.first);
        auto foreach_lambda = [&entity, &e](auto component)
        {
            using component_type = std::remove_pointer_t<decltype(component)>;
            auto &&cdef = component_type::component_def;
            auto c = entity.second.find(cdef.name);
            if (c == entity.second.end()) {
                return;
            }
            if (!(*c).is_object()) {
                throw std::runtime_error("Component " + std::string(cdef.name)
                        + " of " + e.name + " has wrong type");
            }
            auto &componentPtr = e.template get<component_type>();
            try {
                componentPtr = cdef.construct(*c);
            } catch (std::runtime_error err) {
                throw std::runtime_error(e.name + ": " + err.what());
            }
        };
        for_each_type<decltype(foreach_lambda),Components...>(foreach_lambda);
        entities.push_back(e);
    }

    return entities;
}

} // namespace elix

#endif // ELIX_HPP
