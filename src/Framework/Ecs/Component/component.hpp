#pragma once

#include <vector>

#include "../Base/Sparseset.hpp"
#include "../Base/iterator.hpp"
#include "../util.hpp"

namespace ecs::component {

using std::vector;

class group
{
};

template <typename Component, typename Entity>
class componentstorage_iterator
{
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
      : component_iter_(component_ptr), entity_iter_(entity_ptr)
  {
  }

  template <typename Component_Iter, typename Entity_Iter,
            typename = std::void_t<
                util::is_valid_iterator_t<Component_Iter>,
                util::is_valid_iterator_t<Entity_Iter>,
                util::is_same_t<typename Component_Iter::value_type, Component>,
                util::is_same_t<typename Entity_Iter::value_type, Entity>>>
  componentstorage_iterator(Component_Iter C_Iter, Entity_Iter E_Iter)
      : component_iter_(&(*C_Iter)), entity_iter_(&(*E_Iter))
  {
  }

  std::pair<Component&, Entity&> operator*() const
  {
    return std::pair<Component&, Entity&>(*component_iter_, *entity_iter_);
  }

  componentstorage_iterator& operator++()
  {
    ++component_iter_;
    ++entity_iter_;
    return *this;
  }
  componentstorage_iterator operator++(int)
  {
    componentstorage_iterator temp = *this;
    ++component_iter_;
    ++entity_iter_;
    return temp;
  }

  componentstorage_iterator& operator--()
  {
    --component_iter_;
    --entity_iter_;
    return *this;
  }
  componentstorage_iterator operator--(int)
  {
    componentstorage_iterator temp = *this;
    --component_iter_;
    --entity_iter_;
    return temp;
  }

  // distance
  difference_type operator-(const componentstorage_iterator& other)
  {
    return entity_iter_ - other.entity_iter_;
  }

  componentstorage_iterator& operator+=(difference_type n)
  {
    component_iter_ += n;
    entity_iter_ += n;
    return *this;
  }

  componentstorage_iterator& operator-=(difference_type n)
  {
    component_iter_ -= n;
    entity_iter_ -= n;
    return *this;
  }

  componentstorage_iterator operator+(difference_type n)
  {
    return componentstorage_iterator(component_iter_ + n, entity_iter_ + n);
  }

  componentstorage_iterator operator-(difference_type n)
  {
    return componentstorage_iterator(component_iter_ - n, entity_iter_ - n);
  }

  bool operator==(const componentstorage_iterator other)
  {
    return (*entity_iter_) == (*other.entity_iter_);
  }

  bool operator!=(const componentstorage_iterator other)
  {
    return (*entity_iter_) != (*other.entity_iter_);
  }

  bool operator<(const componentstorage_iterator other)
  {
    return (*entity_iter_) < (*other.entity_iter_);
  }

  bool operator>(const componentstorage_iterator other)
  {
    return (*entity_iter_) > (*other.entity_iter_);
  }

private:
  SparseSet_Iterator entity_iter_;
  Iterator<Component> component_iter_;
};

template <typename Component_Type, typename Entity>
class componentstorage
{
  basic_type::SparseSet<Entity> entity_pool;
  vector<Component_Type> components_pool;
  
  using Iterator = componentstorage_iterator<Component_Type, Entity>;

public:
  using value_type = Component_Type;

  using iterator = Iterator;

public:
  Component_Type& operator[](Entity entity)
  {
    return components_pool[entity_pool.get_index(entity)];
  }

  template <typename T>
  void add(Entity entity, T& component)
  {
    entity_pool.emplace_back(entity);
    components_pool.push_back(std::move(component));
  }

  void remove(Entity entity) override
  {
    entity_pool.erase(entity);

    if (static_cast<std::size_t>(entity) != (components_pool.size() + 1)) {
      std::swap(components_pool.at(entity_pool.get_index(entity)),
                components_pool.back());
    }

    components_pool.pop_back();
  }

  Component_Type& at(Entity entity)
  {
    return components_pool.at(entity_pool.get_index(entity));
  }

  [[nodiscard]] std::size_t size() const { return components_pool.size(); }

  Iterator begin() { return {components_pool.begin(), entity_pool.begin()}; }

  Iterator end() { return {components_pool.end(), entity_pool.end()}; }

  void add_batch(Iterator first, Iterator last)
  {
    for (auto it = first; it != last; it++) {
      auto temp = *it;
      components_pool.push_back(temp.first);
      entity_pool.emplace_back(temp.second);
    }
  }
};

class componentTypeManager
{
  std::array<std::size_t, basic_value::MaxComponentTypes> type_to_index;
  std::size_t nextindex = 0;

public:
  componentTypeManager() { type_to_index.fill(static_cast<std::size_t>(-1)); }

  template <typename component>
  size_t get_index()
  {
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
class ComponentStorageWapper
{
  using Storage_ptr = std::shared_ptr<void>;

public:
  ComponentStorageWapper() = default;

  Storage_ptr& operator[](std::size_t count) { return components[count]; }

  const Storage_ptr& operator[](std::size_t count) const
  {
    return components[count];
  }

  [[nodiscard]] std::size_t size() const { return components.size(); }

  void resize(std::size_t count) { components.resize(count); }

  void destory(Entity entity, void* storage, std::size_t type_idx)
  {
    destory_list[type_idx](entity, storage);
  }

  template <typename componentstorage, typename component>
  void set_destory(void (*fn)(Entity, void*), std::size_t type_idx)
  {
    if (destory_list.size() >= type_idx) {
      destory_list.resize(type_idx + 1, nullptr);
    }
    destory_list[type_idx] = fn;
  }

private:
  std::vector<void (*)(Entity, void*)> destory_list;
  std::vector<Storage_ptr> components;
};

}  // namespace ecs::component
