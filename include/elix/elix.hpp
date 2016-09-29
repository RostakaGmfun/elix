#ifndef ELIX_HPP
#define ELIX_HPP

#include <stdexcept>
#include <tuple>
#include <string>
#include <map>
#include <array>
#include <functional>
#include <type_traits>

#include <json.hpp>

namespace elix {

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

    using property_types = std::tuple<Properties...>;
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
    func(static_cast<Component*>(nullptr));
    for_each_component<Func, Components...>(func);
}

template <class TupleType, class CurrentTupleType = TupleType>
auto build_properties_tuple(TupleType &&current_tuple, std::vector<json> &props)
{
    return current_tuple;
}

template <class TupleType, class CurrentTupleType,
        class C = typename
        std::enable_if<!std::is_same<std::decay_t<CurrentTupleType>,
        std::decay_t<TupleType>>::value,
        void>::type>
auto build_properties_tuple(CurrentTupleType &&current_tuple, std::vector<json> &props)
{
    using current_size = std::tuple_size<CurrentTupleType>;
    using property_type = typename std::tuple_element<current_size::value, TupleType>::type;
    auto prop = props.back();
    props.pop_back();
    auto current = std::tuple_cat(current_tuple,std::make_tuple(prop.get<property_type>()));
    return build_properties_tuple<TupleType, CurrentTupleType>(std::move(current), props);
}

template <class TupleType>
auto build_properties_tuple(std::vector<json> &props)
{
    using property_type = typename std::tuple_element<0, TupleType>::type;
    auto prop = props.back();
    props.pop_back();
    auto start = std::make_tuple(prop.get<property_type>());
    return build_properties_tuple<TupleType>(std::move(start), props);
}

template <class Component>
Component *construct_component(json config)
{
    auto &cdef = Component::component_def;
    using cdef_type = typename std::remove_reference_t<decltype(cdef)>;
    std::vector<json> properties;
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
    std::reverse(properties.begin(), properties.end());
    return new Component(
        build_properties_tuple<typename cdef_type::property_types>(properties));
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

} // namespace elix

#endif // ELIX_HPP
