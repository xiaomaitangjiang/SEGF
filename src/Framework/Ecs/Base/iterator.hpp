#pragma once

#include <cstddef>
#include <iterator>
#include <type_traits>

#include "util.hpp"

namespace ecs::iterator {
/**
 * @brief 基础迭代器特性模板类
 * @tparam T 迭代器或元素类型
 * @tparam Enabled 用于SFINAE的启用类型（默认void）
 *
 * 此模板类通过SFINAE技术为迭代器和普通指针类型提供统一的类型特性接口。
 */
template <typename T, typename = void>
class basic_iterator_traits
{
};

/**
 * @brief 迭代器类型的特性特化
 * @tparam T 迭代器类型（必须满足ecs::util::is_iterator_v）
 *
 * 当T为迭代器类型时，直接提取迭代器的原生类型定义。
 */
template <typename T>
class basic_iterator_traits<T, std::enable_if_t<ecs::util::is_iterator_v<T>>>
{
public:
  using iterator_category = typename T::iterator_category;
  using value_type = typename T::value_type;
  using difference_type = typename T::difference_type;
  using pointer = typename T::pointer;
  using reference = typename T::reference;
};

/**
 * @brief 非迭代器类型的特性特化（处理指针类型）
 * @tparam T 元素类型（非迭代器）
 *
 * 当T为非迭代器类型时，默认定义为随机访问迭代器特性，
 * 并适配指针相关的类型定义。
 */
template <typename T>
class basic_iterator_traits<T, std::enable_if_t<!ecs::util::is_iterator_v<T>>>
{
public:
  using iterator_category = std::random_access_iterator_tag;
  using value_type = std::remove_cv_t<T>;
  using difference_type = ptrdiff_t;
  using pointer = T*;
  using reference = T&;
};

template <typename T>
class basic_iterator : basic_iterator_traits<T>
{
public:
  using iterator_category =
      typename basic_iterator_traits<T>::iterator_category;
  using value_type = typename basic_iterator_traits<T>::value_type;
  using difference_type = typename basic_iterator_traits<T>::difference_type;
  using pointer = typename basic_iterator_traits<T>::pointer;
  using reference = typename basic_iterator_traits<T>::reference;

public:
  using Storage = std::conditional_t<util::is_iterator_v<T>, T, T*>;

public:
  explicit basic_iterator() = default;

  explicit basic_iterator(Storage ptr) : ptr_(ptr) {}

  template <typename OtherIter,
            std::enable_if_t<!std::is_same_v<OtherIter, basic_iterator> &&
                                 ecs::util::is_iterator_v<OtherIter>,
                             int> = 0>
  basic_iterator(const OtherIter& other) : ptr_(other)
  {
  }

  value_type& operator*() const { return *ptr_; }

  basic_iterator& operator++()
  {
    ++ptr_;
    return *this;
  }
  basic_iterator operator++(int)
  {
    basic_iterator temp = *this;
    ++ptr_;
    return temp;
  }

  basic_iterator& operator--()
  {
    --ptr_;
    return *this;
  }
  basic_iterator operator--(int)
  {
    basic_iterator temp = *this;
    --ptr_;
    return temp;
  }

  // distance
  difference_type operator-(const basic_iterator other)
  {
    return ptr_ - other.ptr_;
  }

  basic_iterator& operator+=(difference_type n)
  {
    ptr_ += n;
    return *this;
  }

  basic_iterator& operator-=(difference_type n)
  {
    ptr_ -= n;
    return *this;
  }

  basic_iterator operator+(difference_type n)
  {
    return basic_iterator(ptr_ + n);
  }

  basic_iterator operator-(difference_type n)
  {
    return basic_iterator(ptr_ - n);
  }

  bool operator==(const basic_iterator& other) { return ptr_ == other.ptr_; }

  bool operator!=(const basic_iterator& other) { return ptr_ != other.ptr_; }

  bool operator<(const basic_iterator& other) { return ptr_ < other.ptr_; }

  bool operator>(const basic_iterator& other) { return ptr_ > other.ptr_; }

private:
  Storage ptr_;
};

template <typename T>
using Iterator = basic_iterator<T>;

}  // namespace ecs::iterator
