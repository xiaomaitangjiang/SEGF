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
#include "../Entity/Entity.hpp"

/*
Notice:

*/

namespace ecs::registry::basic_registry {
using basic_type::default_type;
using basic_type::uint32;
using Entity = entity::entity<default_type>;
using std::vector;
using ecs::component::componentTypeManager;
using ecs::component::ComponentStorageWapper;
using ecs::component::componentstorage;

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
  // basic_View View

public:
  registry() = default;

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
        components.destory(entity, components[i].get(), i);
      }
      Mask_temp.reset();
      entities[entity] = false;
    }
  }
  
  template<typename component>
  void destory(Entity entity)
  {
    if (!is_valid(entity)) {
      return;
    }
    componentMask& Mask_temp = component_masks[entity];
    std::size_t type_idx = type_manager.get_index<component>();
    Mask_temp.set(type_idx,false);
    components.destory(entity,components[type_idx].get(), std::size_t type_idx)
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

    storage->set_destory(
        [](Entity entity, void* storage_ptr) {
          auto* store =
              static_cast<componentstorage<component, Entity>>(storage_ptr);
          store->remove(entity);
        },
        type_idx);

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

  template <typename component>
  void remove(Entity entity)
  {
    if (!is_valid(entity)) {
      return;
    }
    std::size_t type_index = type_manager.get_index<component>();
    auto& storage = components[type_index];
    if (storage) {
      components.destory(entity, storage.get(), type_index);
    }

    component_masks[entity].reset(type_index);
  }

  void begin() {}

private:
  inline bool is_valid(Entity num)
  {
    return (num <= back_entity) && (entities[num]);
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
