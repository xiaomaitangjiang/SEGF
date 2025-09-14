#pragma once

#include <memory>
#include <vector>

#include "../Base/Sparseset.hpp"
#include "../Base/iterator.hpp"
#include "../util.hpp"
#include "Base/Typeerase.hpp"

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

template <typename Entity>
class basic_componentstorage
{
public:
  using SparseSet_Iterator = basic_type::basic_sparseset_iterator<Entity>;

public:
  virtual ~basic_componentstorage() = 0;
  virtual void remove(Entity entity) = 0;
  [[nodiscard]] virtual std::size_t size() const = 0;
  [[nodiscard]] virtual bool valid(Entity entity) const = 0;
};

template <typename Component_Type, typename Entity>
class componentstorage : public basic_componentstorage<Entity>
{
  basic_type::SparseSet<Entity> entity_pool;
  vector<Component_Type> components_pool;

public:
  using SparseSet_Iterator = basic_type::basic_sparseset_iterator<Entity>;
  using Iterator = componentstorage_iterator<Component_Type, Entity>;
  using value_type = Component_Type;
  using size_type = std::size_t;

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

    if (static_cast<size_type>(entity) != (components_pool.size() + 1)) {
      std::swap(components_pool.at(entity_pool.get_index(entity)),
                components_pool.back());
    }

    components_pool.pop_back();
  }

  Component_Type& at(Entity entity)
  {
    return components_pool.at(entity_pool.get_index(entity));
  }

  [[nodiscard]] size_type size() const override
  {
    return components_pool.size();
  }

  [[nodiscard]] bool valid(Entity entity) const override
  {
    bool invalidity =
        (entity_pool.size() <= entity) && (entity_pool.try_find().has_value());
    return !invalidity;
  }

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
  using Storage = basic_componentstorage<Entity>;
  using Storage_ptr = std::unique_ptr<Storage>;

public:
  using SparseSet_Iterator = basic_type::basic_sparseset_iterator<Entity>;

public:
  ComponentStorageWapper() = default;

  Storage_ptr& operator[](std::size_t count) { return components[count]; }

  const Storage_ptr& operator[](std::size_t count) const
  {
    return components[count];
  }

  [[nodiscard]] std::size_t size() const { return components.size(); }

private:
  std::vector<Storage_ptr> components;
};

}  // namespace ecs::component
