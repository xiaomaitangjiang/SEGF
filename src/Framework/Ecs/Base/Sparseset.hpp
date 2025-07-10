#ifndef _SPARSESET_HPP_
#define _SPARSESET_HPP_

#include <array>
#include <memory>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <vector>

#include "Base/iterator.hpp"
#include "fwd.hpp"

namespace ecs::basic_type {

template <typename Entity>
class basic_sparseset_iterator {
  template <typename T>
  using Iterator = ecs::iterator::basic_iterator<T>;

public:
  using iterator_category = typename Iterator<Entity>::iterator_category;
  using value_type = typename Iterator<Entity>::value_type;
  using difference_type = typename Iterator<Entity>::difference_type;
  using pointer = typename Iterator<Entity>::pointer;
  using reference = typename Iterator<Entity>::reference;

public:
  explicit basic_sparseset_iterator() = default;

  explicit basic_sparseset_iterator(Entity *entity_ptr)
      : Entity_iter(entity_ptr) {}

  template <typename Entity_Iter,
            typename = std::void_t<
                util::is_valid_iterator_t<Entity_Iter>,
                util::is_same_t<typename Entity_Iter::value_type, Entity>>>
  explicit basic_sparseset_iterator(Entity_Iter E_Iter)
      : Entity_iter(&(*E_Iter)) {}

  Entity &operator*() const { return *Entity_iter; }

  basic_sparseset_iterator &operator++() {
    ++Entity_iter;
    return *this;
  }
  basic_sparseset_iterator operator++(int) {
    Iterator<Entity> temp = this->Entity_iter;
    ++Entity_iter;
    return temp;
  }

  basic_sparseset_iterator &operator--() {
    --Entity_iter;
    return *this;
  }
  basic_sparseset_iterator operator--(int) {
    Iterator<Entity> temp = this->Entity_iter;
    --Entity_iter;
    return temp;
  }

  // distance
  difference_type operator-(const basic_sparseset_iterator &other) {
    return Entity_iter - other.Entity_iter;
  }

  basic_sparseset_iterator &operator+=(difference_type n) {
    Entity_iter += n;
    return *this;
  }

  basic_sparseset_iterator &operator-=(difference_type n) {
    Entity_iter -= n;
    return *this;
  }

  basic_sparseset_iterator operator+(difference_type n) {
    return basic_sparseset_iterator(Entity_iter + n);
  }

  basic_sparseset_iterator operator-(difference_type n) {
    return basic_sparseset_iterator(Entity_iter - n);
  }

  bool operator==(const basic_sparseset_iterator &other) {
    return Entity_iter == other.Entity_iter;
  }

  bool operator!=(const basic_sparseset_iterator &other) {
    return Entity_iter != other.Entity_iter;
  }

  bool operator<(const basic_sparseset_iterator &other) {
    return Entity_iter < other.Entity_iter;
  }

  bool operator>(const basic_sparseset_iterator &other) {
    return Entity_iter > other.Entity_iter;
  }

private:
  Iterator<Entity> Entity_iter;
};

/// @brief implementation of sparse
/// @tparam T: type of element
/// @tparam PageSize: size of each page
template <typename T, size_t PageSize,
          typename = std::enable_if_t<std::is_integral_v<T>>>
class basic_sparse {
public:
  basic_sparse() = default;

  basic_sparse(basic_sparse &copy) = delete;

  basic_sparse(basic_sparse &&mov) noexcept : sparse(std::move(mov.sparse)) {}

  basic_sparse &operator=(basic_sparse &copy) = delete;

  basic_sparse &operator=(basic_sparse &&mov) noexcept {
    sparse = std::move(mov.sparse);
  }

  ~basic_sparse() {
    for (auto &elem : sparse) {
      elem.reset();
    }
  }

public:
  /// @brief check the vaildity of value
  /// @param value
  /// @return ture: value is inside the container
  bool contain(const T value) const {
    if (page(value) >= sparse.size()) {
      return false;
    }
    return static_cast<bool>(
        (sparse.at(page(value)).get())->at(in_page(value)));
  }

  /// @brief return the size of sparse
  /// @return size of sparse
  [[nodiscard]] size_t size() const { return sparse.size(); }

  void resize(std::size_t length) {
    if (page(length) + 1 <= sparse.size()) {
      return;
    }
    std::size_t target_len = page(length) + 1;
    for (std::size_t current_len = sparse.size(); target_len > current_len;
         current_len++) {
      sparse.emplace_back(std::unique_ptr<std::array<Type, PageSize>>(
          new std::array<Type, PageSize>));
    }
  }

  /// @brief find a element without check
  /// @param value  element to be found
  /// @return the type of T
  T& at_unsafe(const T value) {
    return *((sparse.at(page(value)))->at(in_page(value)));
  }

  /// @brief find a element with check of range
  /// @param value  element to be found
  /// @return the type of T
  T& at(const T value) {
    if (page(value) >= sparse.size()) {
      throw std::out_of_range("basic_sparse out of range");
    }
    return *((sparse.at(page(value)))->at(in_page(value)));
  }

  /// @brief insert a element into container
  /// @param value
  void insert(const T value, const std::size_t index) {
    if (std::size_t target_len = page(index) +1; target_len > sparse.size()) {
      for (std::size_t current_len = sparse.size(); target_len > current_len;
           current_len++) {
        sparse.emplace_back(std::unique_ptr<std::array<Type, PageSize>>(
            new std::array<Type, PageSize>));
      }
    }
    (sparse.at(page(index)).get())->at(in_page(index)).emplace(value);
  }

  /// @brief shift a element into invalid state
  /// @param index the place of element
  void erase(const T index) {
    if (page(index) >= sparse.size()) {
      return;
    }
    sparse.at(page(index))->at(in_page(index)).reset();
  }

  void clear() { sparse.clear(); }

private:
  size_t page(const T value) const { return value / PageSize; }

  size_t in_page(const T value) const { return value % PageSize; }

  using Type = std::optional<T>;

  std::vector<std::unique_ptr<std::array<Type, PageSize>>> sparse;
};

/// @brief
/// @tparam T
/// @tparam
/// @tparam PageSize
template <typename Entity, size_t PageSize,
          typename = std::enable_if_t<std::is_integral<Entity>::value, Entity>>
class basic_sparseset final {
  template <typename T>
  using Iterator = basic_sparseset_iterator<T>;

public:
  basic_sparseset() = default;

  basic_sparseset(const basic_sparseset &copy) = delete;

  basic_sparseset(basic_sparseset &&mov) noexcept
      : density(std::move(mov.density)), sparse(std::move(mov.sparse)) {}

  basic_sparseset &operator=(const basic_sparseset &copy) = delete;

  basic_sparseset &operator=(basic_sparseset &&mov) noexcept {
    density = std::move(mov.density);
    sparse = std::move(mov.sparse);
    return *this;
  }

  ~basic_sparseset() = default;

public:
  void add(const Entity value) {
    density.push_back(value);
    sparse.insert(density.size() -1, value);
    back_entity = static_cast<std::size_t>(value);
  }

  void erase(const Entity value) {
    if (!sparse.contain(value)) {
      return;
    }

    if (auto &index = sparse.at(value); index == density.size() - 1) {
      sparse.erase(index);
      density.pop_back();
    } else {
      std::swap(density[index], density.back());
      density.pop_back();
      sparse.at_unsafe(density.back()) = index;
      sparse.erase(index);
    }

    if (static_cast<std::size_t>(value) == back_entity) {
      back_entity = density.back();
    }
  }

  std::size_t size() { return density.size(); }

  void resize(std::size_t length) { sparse.resize(length); }

  Entity at(const Entity value) { return density[sparse.at(value)]; }

  Entity find(const Entity value) { return density[sparse.at_unsafe(value)]; }

  std::optional<Entity> try_find(const Entity value) {
    if (!sparse.contain(value)) {
      return std::optional<Entity>();
    }
    return std::make_optional(density[sparse.at_unsafe(value)]);
  }

  std::size_t get_index(const Entity value) const {
    return static_cast<std::size_t>(sparse.at(value));
  }

  bool contain(const Entity value) const { return sparse.contain(value); }

  void clear() {
    density.clear();
    sparse.clear();
  }

  Entity front() { return density.front(); }

  Iterator<Entity> begin() { return Iterator<Entity>(density.begin()); }

  Iterator<Entity> end() { return Iterator<Entity>(density.end()); }

private:
  std::vector<Entity> density;
  ecs::basic_type::basic_sparse<Entity, PageSize> sparse;
  std::size_t back_entity{};
};

template <typename Entity_T>
class SparseSet {
  using sparseset = basic_sparseset<Entity_T, basic_value::ECS_PAGE_SIZE>;

  template <typename T>
  using Iterator = basic_sparseset_iterator<T>;

public:
  friend class basic_sparseset_iterator<Entity_T>;

public:
  SparseSet() : root_sparseset(new sparseset()) {}

  SparseSet(SparseSet &copy) = delete;

  SparseSet(SparseSet &&mov) noexcept
      : root_sparseset(std::move(mov.root_sparseset)) {}

  SparseSet &operator=(SparseSet &copy) = delete;

  SparseSet &operator=(SparseSet &&mov) noexcept {
    root_sparseset.reset(mov.root_sparseset.release());
  }

  ~SparseSet() { root_sparseset.reset(); }

  template <typename... T>
  void emplace_back(const T... args) {
    (root_sparseset->add(args), ...);
  }

  template <typename... T>
  void try_emplace(const Entity_T entity, T... args) {
    if (!(root_sparseset.contain(entity))) {
      emplace_back(entity);
    }
    try_emplace(args...);
  }

  void clear() { root_sparseset.clear(); }

  std::size_t size() { return root_sparseset.size(); }

  void resize(std::size_t length) { root_sparseset.resize(length); }

  template <typename... T>
  void erase(const T... args) {
    (root_sparseset.erase(args), ...);
  }

  Entity_T find(const Entity_T entity) { return root_sparseset.find(entity); }

  std::optional<Entity_T> try_find(const Entity_T entity) {
    return root_sparseset.try_find(entity);
  }

  std::size_t get_index(const Entity_T entity) {
    root_sparseset.get_index(entity);
  }

  auto begin() { return root_sparseset->begin(); }

  auto end() { return root_sparseset->end(); }

private:
  void try_emplace() {}

private:
  std::unique_ptr<sparseset> root_sparseset;
};

}  // namespace ecs::basic_type

#endif
