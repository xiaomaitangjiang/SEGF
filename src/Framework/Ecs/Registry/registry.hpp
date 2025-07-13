#pragma once

#include <any>
#include <array>
#include <bitset>
#include <cstddef>
#include <memory>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>

#include "../Base/Sparseset.hpp"
#include "../Base/ecs_basic_type.hpp"
#include "../Base/fwd.hpp"
#include "../Base/iterator.hpp"
#include "../Entity/Entity.hpp"
#include "../util.hpp"

/*
Notice:

*/

namespace ecs::registry::basic_registry {
using basic_type::default_type;
using basic_type::uint32;
using Entity = entity::entity<default_type>;
using std::vector;

template <typename Component, typename Entity>
class componentstorage_iterator {
  template <typename T>
  using Iterator = ecs::iterator::basic_iterator<T>;

  using SparseSet_Iterator = ecs::basic_type::basic_sparseset_iterator<Entity>;

public:
  using iterator_category = typename Iterator<Entity>::iterator_category;
  using value_type = typename Iterator<Entity>::value_type;
  using difference_type = typename Iterator<Entity>::difference_type;
  using pointer = typename Iterator<Entity>::pointer;
  using reference = typename Iterator<Entity>::reference;

public:
  explicit componentstorage_iterator() = default;
  componentstorage_iterator(Component* component_ptr, Entity* entity_ptr)
      : component_iter_(component_ptr), entity_iter_(entity_ptr) {}

  template <typename Component_Iter, typename Entity_Iter,
            typename = std::void_t<
                util::is_valid_iterator_t<Component_Iter>,
                util::is_valid_iterator_t<Entity_Iter>,
                util::is_same_t<typename Component_Iter::value_type, Component>,
                util::is_same_t<typename Entity_Iter::value_type, Entity>>>
  componentstorage_iterator(Component_Iter C_Iter, Entity_Iter E_Iter)
      : component_iter_(&(*C_Iter)), entity_iter_(&(*E_Iter)) {}

  std::pair<Component&, Entity&> operator*() const {
    return std::pair<Component&, Entity&>(*component_iter_, *entity_iter_);
  }

  componentstorage_iterator& operator++() {
    ++component_iter_;
    ++entity_iter_;
    return *this;
  }
  componentstorage_iterator operator++(int) {
    componentstorage_iterator temp = *this;
    ++component_iter_;
    ++entity_iter_;
    return temp;
  }

  componentstorage_iterator& operator--() {
    --component_iter_;
    --entity_iter_;
    return *this;
  }
  componentstorage_iterator operator--(int) {
    componentstorage_iterator temp = *this;
    --component_iter_;
    --entity_iter_;
    return temp;
  }

  // distance
  difference_type operator-(const componentstorage_iterator& other) {
    return entity_iter_ - other.entity_iter_;
  }

  componentstorage_iterator& operator+=(difference_type n) {
    component_iter_ += n;
    entity_iter_ += n;
    return *this;
  }

  componentstorage_iterator& operator-=(difference_type n) {
    component_iter_ -= n;
    entity_iter_ -= n;
    return *this;
  }

  componentstorage_iterator operator+(difference_type n) {
    return componentstorage_iterator(component_iter_ + n, entity_iter_ + n);
  }

  componentstorage_iterator operator-(difference_type n) {
    return componentstorage_iterator(component_iter_ - n, entity_iter_ - n);
  }

  bool operator==(const componentstorage_iterator other) {
    return (*entity_iter_) == (*other.entity_iter_);
  }

  bool operator!=(const componentstorage_iterator other) {
    return (*entity_iter_) != (*other.entity_iter_);
  }

  bool operator<(const componentstorage_iterator other) {
    return (*entity_iter_) < (*other.entity_iter_);
  }

  bool operator>(const componentstorage_iterator other) {
    return (*entity_iter_) > (*other.entity_iter_);
  }

private:
  SparseSet_Iterator entity_iter_;
  Iterator<Component> component_iter_;
};

template <typename Component_Type, typename Entity>
class componentstorage {
  basic_type::SparseSet<Entity> entity_pool;
  vector<Component_Type> components_pool;

  template <typename T1, typename T2>
  using Iterator = componentstorage_iterator<T1, T2>;

public:
  Component_Type& operator[](Entity entity) {
    return components_pool[entity_pool.get_index(entity)];
  }

  template <typename T>
  void add(Entity entity, T& component) {
    entity_pool.emplace_back(entity);
    components_pool.push_back(std::move(component));
  }

  void remove(Entity entity) override {
    entity_pool.erase(entity);

    if (static_cast<std::size_t>(entity) != (components_pool.size() + 1)) {
      std::swap(components_pool.at(entity_pool.get_index(entity)),
                components_pool.back());
    }

    components_pool.pop_back();
  }

  Component_Type& at(Entity entity) {
    return components_pool.at(entity_pool.get_index(entity));
  }

  [[nodiscard]] std::size_t size() const { return components_pool.size(); }

  Iterator<Component_Type, Entity> begin() {
    return {components_pool.begin(), entity_pool.begin()};
  }

  Iterator<Component_Type, Entity> end() {
    return {components_pool.end(), entity_pool.end()};
  }

  void add_batch(Iterator<Component_Type, Entity> first,
                 Iterator<Component_Type, Entity> last) {
    for (auto it = first; it != last; it++) {
      auto temp = *it;
      components_pool.push_back(temp.first);
      entity_pool.emplace_back(temp.second);
    }
  }
};

class componentTypeManager {
  std::array<std::size_t, basic_value::MaxComponentTypes> type_to_index;
  std::size_t nextindex = 0;

public:
  componentTypeManager() { type_to_index.fill(static_cast<std::size_t>(-1)); }

  template <typename component>
  size_t get_index() {
    size_t hash_index = typeid(component).hash_code();
    for (std::size_t i : type_to_index) {
      if (i == hash_index) {
        return i;
      }
    }

    if (nextindex > basic_value::MaxComponentTypes) {
      throw std::runtime_error("Maximum component types exceeded");
    }

    type_to_index[nextindex] = hash_index;

    return nextindex++;
  }

  [[nodiscard]] std::size_t count() const { return nextindex; }
};

template <typename Entity>
class ComponentStorageWapper {
  using Storage_ptr = std::shared_ptr<void>;

public:
  ComponentStorageWapper() = default;

  Storage_ptr& operator[](std::size_t count) { return components[count]; }

  const Storage_ptr& operator[](std::size_t count) const {
    return components[count];
  }

  [[nodiscard]] std::size_t size() const { return components.size(); }

  void resize(std::size_t count) { components.resize(count); }

  void destory(Entity entity, void* storage, std::size_t type_idx) {
    destory_list[type_idx](entity, storage);
  }

  template <typename componentstorage, typename component>
  void set_destory(void (*fn)(Entity, void*), std::size_t type_idx) {
    if (destory_list.size() >= type_idx) {
      destory_list.resize(type_idx + 1, nullptr);
    }
    destory_list[type_idx] = fn;
  }

private:
  std::vector<void (*)(Entity, void*)> destory_list;
  std::vector<Storage_ptr> components;
};

using basic_type::uint8;
using basic_value::MaxComponentTypes;

template <typename Entity,
          typename = std::enable_if_t<std::is_integral_v<Entity>>>
class registry {
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

  Entity create() {
    if (back_entity >= entities.size()) {
      entities.resize(++back_entity, false);
      component_masks.resize(back_entity);
    }

    entities[back_entity] = true;

    return back_entity;
  }

  void destory(Entity entity) {
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

  template <typename component, typename... Args>
  component& add(Entity entity, Args&&... args) {
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
  component& get(Entity entity) {
    if (!is_valid(entity)) {
      throw std::runtime_error("entity out of range");
    }
    std::size_t type_index = type_manager.get_index<component>();
    auto* storage = static_cast<componentstorage<component, Entity>>(
        components[type_index].get());
    return storage->at(entity);
  }

  template <typename component>
  void remove(Entity entity) {
    if (!is_valid(entity)) {
      return;
    }
    std::size_t type_index = type_manager.get_index<component>();
    auto& storage = components[type_index];
    if (storage) {
      components.destory(entity, storage.get(),type_index);
    }

    component_masks[entity].reset(type_index);
  }

private:

  inline bool is_valid(Entity num) {
    return (num <= back_entity) && (entities[num]);
  }

  template <typename component>
  componentstorage<component, Entity>* get_storage_ptr() {
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
  const componentstorage<component, Entity>* get_storage_ptr() const {
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
