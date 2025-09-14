#pragma once

#include <bitset>
#include <cstddef>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

#include "../Base/ecs_basic_type.hpp"
#include "../Base/fwd.hpp"
#include "../Component/component.hpp"
#include "../Ensemble/Ensemble.hpp"
#include "../Entity/Entity.hpp"
#include "../View/View.hpp"

/*
Notice:

*/

namespace ecs::registry::basic_registry {
using basic_type::default_type;
using basic_type::uint32;
using Entity = entity::entity<default_type>;
using ecs::component::componentstorage;
using ecs::component::ComponentStorageWapper;
using ecs::component::componentTypeManager;
using std::vector;

using basic_type::uint8;
using basic_value::MaxComponentTypes;

template <typename Entity,
          typename = std::enable_if_t<std::is_integral_v<Entity>>>
class registry
{
  using componentMask = std::bitset<MaxComponentTypes>;
  vector<componentMask> component_masks;

  componentTypeManager type_manager;

  ComponentStorageWapper<Entity> components;

  Entity back_entity{};
  vector<uint8> entities;  // a replacement of vector<bool>

public:
public:
  registry() = default;

public:
  template <typename T>
  using View = view::basic_view<vector, T>;

  // todo
  /*
    template<typename... Args>
    using View=view::basic_view<,>
  */
public:
  Entity create()
  {
    if (back_entity >= entities.size()) {
      entities.resize(++back_entity, false);
      component_masks.resize(back_entity);
    }

    entities[back_entity] = true;

    return back_entity;
  }

  void destory(Entity entity)
  {
    if (!is_valid(entity)) {
      return;
    }
    componentMask& Mask_temp = component_masks[entity];
    for (std::size_t i = 0; i <= MaxComponentTypes; i += 1) {
      if (Mask_temp.test(i)) {
        components.remove(entity, i);
      }
      Mask_temp.reset();
      entities[entity] = false;
    }
  }

  template <typename component>
  void remove(Entity entity)
  {
    if (!is_valid(entity)) {
      return;
    }
    componentMask& Mask_temp = component_masks[entity];
    std::size_t type_idx = type_manager.get_index<component>();
    Mask_temp.set(type_idx, false);
    components.remove(entity, type_idx);
  }

  template <typename component, typename... Args>
  component& add(Entity entity, Args&&... args)
  {
    auto* storage{get_storage_ptr<component>()};
    if (!storage) {
      throw std::runtime_error("component storage creation failed");
    }
    std::size_t type_idx = type_manager.get_index<component>();
    component_masks[entity].set(type_idx);

    return storage->add(entity, component{std::forward<Args>(args)...});
  }

  template <typename component>
  component& get(Entity entity)
  {
    if (!is_valid(entity)) {
      throw std::runtime_error("entity out of range");
    }
    std::size_t type_index = type_manager.get_index<component>();
    auto* storage = static_cast<componentstorage<component, Entity>>(
        components[type_index].get());
    return storage->at(entity);
  }

  template <typename Component>
  View<Component> view()
  {
    return View<Component>(
        *(components[type_manager.get_index<Component>()].get()));
  }

  template <typename... Components>
  View<ensemble::Ensemble<View, Components...>> view()
  {
    return View<ensemble::Ensemble<View, Components...>>(
        *((components[type_manager.get_index<Components>()].get()), ...));
  }

private:
  inline bool is_valid(Entity num)
  {
    return (num <= back_entity) && (entities[num]);
  }

  void remove(Entity entity, std::size_t type_idx)
  {
    (components[type_idx].get())->remove(entity);
  }

  template <typename component>
  componentstorage<component, Entity>* get_storage_ptr()
  {
    std::size_t index = type_manager.get_index<component>();
    if (index >= basic_value::MaxComponentTypes) {
      return nullptr;
    }
    if (index >= components.size()) {
      components.resize(index + 1);
    }
    if (!components[index]) {
      components[index] =
          std::make_unique<componentstorage<component, Entity>>();
    }

    return static_cast<componentstorage<component, Entity>*>(
        components[index].get());
  }

  template <typename component>
  const componentstorage<component, Entity>* get_storage_ptr() const
  {
    std::size_t index = type_manager.get_index<component>();
    if (index >= basic_value::MaxComponentTypes) {
      return nullptr;
    }
    if (index >= components.size()) {
      return nullptr;
    }
    if (!components[index]) {
      return nullptr;
    }
    return static_cast<const componentstorage<component, Entity>*>(
        components[index].get());
  }
};

}  // namespace ecs::registry::basic_registry
