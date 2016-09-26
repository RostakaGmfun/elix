#ifndef ECF_HPP
#define ECF_HPP

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <tuple>
#include <string>
#include <map>
#include <array>
#include <functional>
#include <experimental/any>
#include <type_traits>

#include <json.hpp>

namespace ecf {

using json = nlohmann::json;

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

template <class ... Properties>
struct component_def
{
    constexpr component_def(const char *name,
            std::array<const char*, sizeof...(Properties)> &&property_names):
        name(name),
        property_names(property_names)
    {}
    const char *name;
    std::array<const char*, sizeof...(Properties)> property_names;

    using property_tuple = std::tuple<Properties...>;
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

template <class Func>
void for_each_component(Func func)
{}

template <class Func, class Component, class ... Components>
void for_each_component(Func func)
{
    func(Component{});
    for_each_component<Func, Components...>(func);
}

template <class Component>
Component *construct_component(json config)
{
    std::vector<json> properties;
    auto &&cdef = Component::component_def;
    using component_def_type = decltype(cdef);
    typename component_def_type::property_types props;
    // check if all properties are defined
    for (const auto &prop : cdef.property_names) {
        auto p = config.find(prop);
        if (p == config.end()) {
            // Ooops, bad thing
            // There is currently no way to define default property value
            throw std::runtime_error("Property " + std::string(prop) +
                    " is undefined in component " + std::string(cdef.name));
        }
        properties.push_back(*p);
    }

    // TODO
    return new Component{};
}

template <class ... Components>
auto load(const std::string &jsonStr)
{
    auto&& json = json::parse(jsonStr);
    entity_list<Components...> entities;

    for (const auto &entity: json) {
        auto e = make_entity<Components ...>(entity["__name"]);
        auto foreach_lambda = [&entity, &e](auto component)
        {
            using component_type = decltype(component);
            auto &&cdef = component_type::component_def;
            auto c = entity.find(cdef.name);
            if (c == entity.end()) {
                return;
            }
            if (!(*c).is_object()) {
                throw std::runtime_error("Component " + std::string(cdef.name)
                        + " of " + e.name + " has wrong type");
            }
            auto &componentPtr = e.template get<component_type>();
            try {
                componentPtr = construct_component<component_type>(*c);
            } catch (std::runtime_error err) {
                throw std::runtime_error(e.name + ": " + err.what());
            }
        };
        for_each_component<decltype(foreach_lambda),Components...>(foreach_lambda);
        entities.push_back(e);
    }

    return entities;
}

} // namespace ecf

#endif // ECF_HPP
