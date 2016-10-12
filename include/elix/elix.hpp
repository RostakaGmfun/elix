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

// Brilliant idea from http://stackoverflow.com/a/16000226
template <class C, typename = int>
struct is_component:
    std::false_type {};

template <class C>
struct is_component<C, decltype((void)C::component_def, 0)>:
    std::true_type {};


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

    void encode(json &output, const Component *cptr) const
    {
        encode_impl(output, cptr, std::index_sequence_for<Properties...>{});
    }

    auto decode(const json &props, Component *cptr) const
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
        decode_impl(props, *cptr, std::index_sequence_for<Properties...>{});
        return cptr;
    }

private:
    template <std::size_t ... Is>
    void encode_impl(json &out, const Component *c, std::index_sequence<Is...>) const
    {
        (void)std::initializer_list<int>{(encode_property(out[property_names[Is]],
                c->*std::get<Is>(properties)), 0)...};
    }

    template <class T, typename std::enable_if<is_component<T>::value>::type* = nullptr>
    void encode_property(json &data, const T &prop) const
    {
        T::component_def.encode(data, &prop);
    }

    template <class T, typename std::enable_if<!is_component<T>::value>::type* = nullptr>
    void encode_property(json &data, const T &prop) const
    {
        data = prop;
    }

    template <std::size_t ... Is>
    void decode_impl(const json &props, Component &c, std::index_sequence<Is...>) const
    {
        (void)std::initializer_list<int>{
            (decode_property(props[property_names[Is]], c.*std::get<Is>(properties)), 0)...};
    }

    template <class T, typename std::enable_if<is_component<T>::value>::type* = nullptr>
    void decode_property(const json &data, T &property) const
    {
        T::component_def.decode(data, &property);
    }

    template <class T, typename std::enable_if<!is_component<T>::value>::type* = nullptr>
    void decode_property(const json &data, T &property) const
    {
        property = data;
    }

};

template <class ... Components>
struct entity
{
    std::tuple<Components*...> components;
    std::string name;

    entity(std::string name): name(std::move(name))
    {}

    template <class Component>
    Component* &get()
    {
        return std::get<Component*>(components);
    }
};

template <class ... Components>
auto make_entity(const std::string &name)
{
    entity<Components...> e(name);
    return e;
}

template <class ... Components>
using entity_list = std::vector<entity<Components...>>;

template <class Func, class ... C>
void for_each_type(Func func)
{
    (void)std::initializer_list<int> {(func(static_cast<C*>(nullptr)), 0)...};
}

template <class Func, class ... C>
void for_each_value(const std::tuple<C...> &values, Func f)
{
    for_each_value_impl(values, f, std::index_sequence_for<C...>{});
}

template <class Func, class Tuple, size_t ... Is>
void for_each_value_impl(const Tuple &tuple, Func f, std::index_sequence<Is...>)
{
    (void)std::initializer_list<int>{(f(std::get<Is>(tuple)), 0)...};
}

template <class ... Components>
auto decode(const std::string &jsonStr)
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
                componentPtr = cdef.decode(*c, nullptr);
            } catch (std::runtime_error err) {
                throw std::runtime_error(e.name + ": " + err.what());
            }
        };
        for_each_type<decltype(foreach_lambda),Components...>(foreach_lambda);
        entities.push_back(e);
    }

    return entities;
}

template <class ... Components>
auto encode(const entity_list<Components...> &entities)
{
    json output;
    for (const auto &entity : entities) {
        json e;
        auto foreach_lambda = [&e](const auto &component)
        {
            if (component != nullptr) {
                json c;
                component->component_def.encode(c, component);
                e[component->component_def.name] = c;
            }
        };
        for_each_value(entity.components, foreach_lambda);
        output[entity.name] = e;
    }

    return output;
}

} // namespace elix

#endif // ELIX_HPP
