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

template <class>
struct pointer_to_member;

template <class C, class T>
struct pointer_to_member<T C::*>
{
    using class_type = C;
    using member_type = T;
};

template <class C, typename = int>
struct is_component:
    std::false_type {};

template <class C>
struct is_component<C, decltype((void)C::component_def, 0)>:
    std::true_type {};

template <class Component, class = int>
struct property_constructor
{
    property_constructor(const json &data)
    {

    }

    operator()()
    {

    }
};

template <class Component, class ... Properties>
struct component_def
{
    template <class Property>
    using property_t = std::pair<const char*, Property Component::*>;

    using property_types = std::tuple<Properties...>;
    using property_names_t = std::array<const char*, sizeof...(Properties)>;

    const char *name;
    std::tuple<Properties Component::*...> properties;
    property_names_t property_names;

    constexpr component_def(const char *name, property_t<Properties>&& ... properties):
        name(name),
        properties(std::make_tuple(properties.second...)),
        property_names({properties.first...})
    {}

    auto construct(const json &props, Component *cptr) const
    {
        // check if all properties are defined
        for (auto prop : property_names) {
            auto p = props.find(prop);
            if (p == props.end()) {
                // Ooops, bad thing
                // There is currently no way to define default property value
                throw std::runtime_error("Property " + std::string(prop) +
                        " is undefined in component " + std::string(name));
            }
        }

        if (cptr == nullptr) {
            cptr = new Component;
        }
        construct_impl(props, *cptr, std::index_sequence_for<Properties...>{});
        return cptr;
    }

private:
    template <std::size_t ... Is>
    void construct_impl(const json &props, Component & c, std::index_sequence<Is...>) const
    {
        (void)std::initializer_list<int>{
            (construct_property(props[property_names[Is]], std::get<Is>(properties), &c), 0)...};
    }

    template <class T, class = int>
    void construct_property(const json &props, T &property, Component *c) const
    {
        c->*property = props;
    }

    template <class T, class C = std::enable_if_t<is_component<T>::value, T>>
    void construct_property(const json &props, C &property, Component *c) const
    {
        C::component_def.construct(&(c->*property));
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
                componentPtr = cdef.construct(*c, nullptr);
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
