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
class basic_sparseset_iterator
{
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
      : Entity_iter(entity_ptr)
  {
  }

  template <typename Entity_Iter,
            typename = std::void_t<
                util::is_valid_iterator_t<Entity_Iter>,
                util::is_same_t<typename Entity_Iter::value_type, Entity>>>
  explicit basic_sparseset_iterator(Entity_Iter E_Iter)
      : Entity_iter(&(*E_Iter))
  {
  }

  Entity &operator*() const { return *Entity_iter; }

  basic_sparseset_iterator &operator++()
  {
    ++Entity_iter;
    return *this;
  }
  basic_sparseset_iterator operator++(int)
  {
    Iterator<Entity> temp = this->Entity_iter;
    ++Entity_iter;
    return temp;
  }

  basic_sparseset_iterator &operator--()
  {
    --Entity_iter;
    return *this;
  }
  basic_sparseset_iterator operator--(int)
  {
    Iterator<Entity> temp = this->Entity_iter;
    --Entity_iter;
    return temp;
  }

  // distance
  difference_type operator-(const basic_sparseset_iterator &other)
  {
    return Entity_iter - other.Entity_iter;
  }

  basic_sparseset_iterator &operator+=(difference_type n)
  {
    Entity_iter += n;
    return *this;
  }

  basic_sparseset_iterator &operator-=(difference_type n)
  {
    Entity_iter -= n;
    return *this;
  }

  basic_sparseset_iterator operator+(difference_type n)
  {
    return basic_sparseset_iterator(Entity_iter + n);
  }

  basic_sparseset_iterator operator-(difference_type n)
  {
    return basic_sparseset_iterator(Entity_iter - n);
  }

  bool operator==(const basic_sparseset_iterator &other)
  {
    return Entity_iter == other.Entity_iter;
  }

  bool operator!=(const basic_sparseset_iterator &other)
  {
    return Entity_iter != other.Entity_iter;
  }

  bool operator<(const basic_sparseset_iterator &other)
  {
    return Entity_iter < other.Entity_iter;
  }

  bool operator>(const basic_sparseset_iterator &other)
  {
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
class basic_sparse
{
public:
  basic_sparse() = default;

  basic_sparse(basic_sparse &copy) = delete;

  basic_sparse(basic_sparse &&mov) noexcept : sparse(std::move(mov.sparse)) {}

  basic_sparse &operator=(basic_sparse &copy) = delete;

  basic_sparse &operator=(basic_sparse &&mov) noexcept
  {
    sparse = std::move(mov.sparse);
  }

  ~basic_sparse()
  {
    for (auto &elem : sparse) {
      elem.reset();
    }
  }

public:
  /// @brief check the vaildity of value
  /// @param value
  /// @return ture: value is inside the container
  bool contain(const T value) const
  {
    if (page(value) >= sparse.size()) {
      return false;
    }
    return static_cast<bool>(
        (sparse.at(page(value)).get())->at(in_page(value)));
  }

  /// @brief return the size of sparse
  /// @return size of sparse
  [[nodiscard]] size_t size() const { return sparse.size(); }

  void resize(std::size_t length)
  {
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
  T &at_unsafe(const T value)
  {
    return *((sparse.at(page(value)))->at(in_page(value)));
  }

  /// @brief find a element with check of range
  /// @param value  element to be found
  /// @return the type of T
  T &at(const T value)
  {
    if (page(value) >= sparse.size()) {
      throw std::out_of_range("basic_sparse out of range");
    }
    return *((sparse.at(page(value)))->at(in_page(value)));
  }

  /// @brief insert a element into container
  /// @param value
  void insert(const T value, const std::size_t index)
  {
    if (std::size_t target_len = page(index) + 1; target_len > sparse.size()) {
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
  void erase(const T index)
  {
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

/**
 * @class basic_sparseset
 * @brief Sparse set data structure for efficient entity storage and lookup
 *
 * Implements a sparse set container optimized for integral entity IDs with:
 *  - O(1) insertion and deletion
 *  - O(1) membership checks
 *  - Cache-friendly dense storage
 *  - Stable references to elements
 *
 * @tparam Entity    Integral type representing entity IDs
 * @tparam PageSize  Size of memory pages for sparse storage
 * @tparam Unused    SFINAE enabler (ensures Entity is integral type)
 *
 * @note The container maintains two primary structures:
 *       1. density: Contiguous vector of stored entities
 *       2. sparse:  Paged index mapping entities to density positions
 */
template <typename Entity, size_t PageSize,
          typename = std::enable_if_t<std::is_integral<Entity>::value, Entity>>
class basic_sparseset final
{
  template <typename T>
  using Iterator = basic_sparseset_iterator<T>;

public:
  basic_sparseset() = default;

  basic_sparseset(const basic_sparseset &copy) = delete;

  /**
   * @brief Move constructor
   * @param mov Source sparse set to move from
   * @post Source sparse set is left in valid but unspecified state
   */
  basic_sparseset(basic_sparseset &&mov) noexcept
      : density(std::move(mov.density)), sparse(std::move(mov.sparse))
  {
  }

  basic_sparseset &operator=(const basic_sparseset &copy) = delete;

  /**
   * @brief Move assignment operator
   * @param mov Source sparse set to move from
   * @return Reference to this object
   * @post Source sparse set is left in valid but unspecified state
   */
  basic_sparseset &operator=(basic_sparseset &&mov) noexcept
  {
    density = std::move(mov.density);
    sparse = std::move(mov.sparse);
    return *this;
  }

  ~basic_sparseset() = default;

public:
  /**
   * @brief Add an entity to the sparse set
   * @param value Entity ID to add
   * @post Entity is appended to density vector and indexed in sparse structure
   */
  void add(const Entity value)
  {
    density.push_back(value);
    sparse.insert(density.size() - 1, value);
    back_entity = static_cast<std::size_t>(value);
  }

  /**
   * @brief Remove an entity from the sparse set
   * @param value Entity ID to remove
   * @post If entity exists, it's removed with constant-time swap-pop
   * @note Maintains container integrity through index updates
   */
  void erase(const Entity value)
  {
    if (!sparse.contain(value)) {
      return;
    }

    if (auto &index = sparse.at(value); index == density.size() - 1) {
      sparse.erase(index);
      density.pop_back();
    }
    else {
      std::swap(density[index], density.back());
      density.pop_back();
      sparse.at_unsafe(density.back()) = index;
      sparse.erase(index);
    }

    if (static_cast<std::size_t>(value) == back_entity) {
      back_entity = density.back();
    }
  }

  /// @return Number of entities in the container
  std::size_t size() { return density.size(); }

  /**
   * @brief Resize the sparse index
   * @param length New size for sparse index
   * @note Affects sparse storage capacity, not density size
   */
  void resize(std::size_t length) { sparse.resize(length); }

  /**
   * @brief Access entity in dense storage with bounds checking
   * @param value Entity ID to look up
   * @return Entity reference from dense storage
   * @throw May throw if sparse index is out of bounds
   */
  Entity at(const Entity value) { return density[sparse.at(value)]; }

  /**
   * @brief Access entity in dense storage without bounds checking
   * @param value Entity ID to look up
   * @return Entity reference from dense storage
   * @warning Undefined behavior if entity doesn't exist
   */
  Entity find(const Entity value) { return density[sparse.at_unsafe(value)]; }

  /**
   * @brief Safe entity lookup
   * @param value Entity ID to look up
   * @return Optional containing entity if found, empty otherwise
   */
  std::optional<Entity> try_find(const Entity value)
  {
    if (!sparse.contain(value)) {
      return std::optional<Entity>();
    }
    return std::make_optional(density[sparse.at_unsafe(value)]);
  }

  /**
   * @brief Get dense index position of entity
   * @param value Entity ID to locate
   * @return Index in density vector
   */
  std::size_t get_index(const Entity value) const
  {
    return static_cast<std::size_t>(sparse.at(value));
  }

  /**
   * @brief Check if entity exists in container
   * @param value Entity ID to check
   * @return true if entity exists, false otherwise
   */
  bool contain(const Entity value) const { return sparse.contain(value); }

  /// Clear all container data
  void clear()
  {
    density.clear();
    sparse.clear();
  }

  /// @return First entity in dense storage
  Entity front() { return density.front(); }

  /// @return Iterator to beginning of dense storage
  Iterator<Entity> begin() { return Iterator<Entity>(density.begin()); }

  /// @return Iterator to end of dense storage
  Iterator<Entity> end() { return Iterator<Entity>(density.end()); }

private:
  std::vector<Entity> density;
  ecs::basic_type::basic_sparse<Entity, PageSize> sparse;
  std::size_t back_entity{};
};

template <typename Entity_T>
class SparseSet
{
  using sparseset = basic_sparseset<Entity_T, basic_value::ECS_PAGE_SIZE>;

  template <typename T>
  using Iterator = basic_sparseset_iterator<T>;

public:
  friend class basic_sparseset_iterator<Entity_T>;

public:
  SparseSet() : root_sparseset(new sparseset()) {}

  SparseSet(SparseSet &copy) = delete;

  SparseSet(SparseSet &&mov) noexcept
      : root_sparseset(std::move(mov.root_sparseset))
  {}

  SparseSet &operator=(SparseSet &copy) = delete;

  SparseSet &operator=(SparseSet &&mov) noexcept
  {
    root_sparseset = std::move(mov.root_sparseset);
    return *this;
  }

  ~SparseSet()=default;

  /**
   * @brief Add multiple entities in batch
   * @tparam T Entity ID types (must be convertible to Entity_T)
   * @param args Entities to add
   * @post All arguments are inserted into the sparse set
   */
  template <typename... T>
  void emplace_back(const T... args)
  {
    (root_sparseset->add(args), ...);
  }

  /**
   * @brief Conditionally add entities if they don't exist
   * @tparam T Entity ID types
   * @param entity First entity to conditionally add
   * @param args Remaining entities to conditionally add
   * @note Recursively processes all arguments
   */
  template <typename... T>
  void try_emplace(const Entity_T entity, T... args)
  {
    if (!(root_sparseset->contain(entity))) {
      emplace_back(entity);
    }
    try_emplace(args...);
  }

  /// Clear all entities from the sparse set
  void clear() { root_sparseset->clear(); }

  /// @return Current number of entities in the set
  std::size_t size() { return root_sparseset->size(); }

  /**
   * @brief Resize underlying sparse storage
   * @param length New capacity for sparse index
   */
  void resize(std::size_t length) { root_sparseset->resize(length); }

  /**
   * @brief Remove multiple entities in batch
   * @tparam T Entity ID types
   * @param args Entities to remove
   */
  template <typename... T>
  void erase(const T... args)
  {
    (root_sparseset->erase(args), ...);
  }

  /**
   * @brief Direct entity lookup
   * @param entity Entity ID to find
   * @return Reference to the entity in dense storage
   * @warning Undefined behavior if entity doesn't exist
   */
  Entity_T find(const Entity_T entity) { return root_sparseset->find(entity); }

  /**
   * @brief Safe entity lookup
   * @param entity Entity ID to find
   * @return Optional containing entity if found, empty otherwise
   */
  std::optional<Entity_T> try_find(const Entity_T entity)
  {
    return root_sparseset->try_find(entity);
  }

  /**
   * @brief Get dense index position of entity
   * @param entity Entity ID to locate
   * @return Index in dense storage
   */
  std::size_t get_index(const Entity_T entity)
  {
    root_sparseset->get_index(entity);
  }

  /// @return Iterator to beginning of dense storage
  auto begin() { return root_sparseset->begin(); }

  /// @return Iterator to end of dense storage
  auto end() { return root_sparseset->end(); }

private:
  /// Terminator for recursive try_emplace
  void try_emplace() {}

private:
  std::unique_ptr<sparseset> root_sparseset;
};

}  // namespace ecs::basic_type

#endif
