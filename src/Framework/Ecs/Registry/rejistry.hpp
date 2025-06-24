#pragma once

#include <cstddef>
#include <type_traits>
#include <utility>
#include <vector>

#include "../Base/Sparseset.hpp"
#include "../Base/ecs_basic_type.hpp"
#include "../Entity/Entity.hpp"
#include "../util.hpp"
#include "Base/iterator.hpp"

/*
Notice:
未实现basic_sparseset_iterator，SparseSet，componentstorage的具体实现可能变动
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

  std::pair<Component, Entity>& operator*() const {
    return std::make_pair(*component_iter_,*entity_iter_);
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
  T& add(Entity entity, T& component) {
    entity_pool.emplace_back(entity);
    components_pool.push_back(std::move(component));
  }

  void remove(Entity entity) {
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

  Iterator<Component_Type, Entity> begin() {
    return 
        {components_pool.begin(), entity_pool.begin()};
  }

  Iterator<Component_Type, Entity> end() {
    return {components_pool.end(),entity_pool.end()};
  }

  void add_batch(Iterator<Component_Type, Entity> first,
                 Iterator<Component_Type, Entity> last) {
    for (auto it = first; it != last; it++) {
      auto temp = *it;
      components_pool.push_back(temp.first);
      entity_pool.emplace_back(temp.second);
    }
  }
  /*
    template <
        typename It, typename Entity_T, typename Component_T,
        typename = std::void_t<std::enable_if_t<util::is_valid_iterator_v<It>>>>
    void add_batch(It first, It last) {
      const std::size_t batch_len = std::distance(first, last);
      if (batch_len == 0) {
        return;
      }
      std::size_t valid_range = entity_pool.size();
      for (auto it = first; it != last; ++it) {
      }
    }*/
};

}  // namespace ecs::registry::basic_registry
