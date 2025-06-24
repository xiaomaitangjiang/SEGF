#pragma once

#include <cstddef>
#include <iterator>
#include <type_traits>

#include "util.hpp"

namespace ecs::iterator {
template <typename T, typename = void>
class basic_iterator_traits {};

template <typename T>
class basic_iterator_traits<T,
                            std::enable_if_t<ecs::util::is_iterator_v<T>>> {
public:
  using iterator_category = typename T::iterator_category;
  using value_type = typename T::value_type;
  using difference_type = typename T::difference_type;
  using pointer = typename T::pointer;
  using reference = typename T::reference;
};

template <typename T>
class basic_iterator_traits<T,
                            std::enable_if_t<!ecs::util::is_iterator_v<T>>> {
public:
  using iterator_category = std::random_access_iterator_tag;
  using value_type = std::remove_cv_t<T>;
  using difference_type = ptrdiff_t;
  using pointer = T*;
  using reference = T&;
};

template <typename T>
class basic_iterator:basic_iterator_traits<T> {
public:
  using iterator_category =typename basic_iterator_traits<T>::iterator_category;
  using value_type = typename basic_iterator_traits<T>::value_type;
  using difference_type = typename basic_iterator_traits<T>::difference_type;
  using pointer = typename basic_iterator_traits<T>::pointer;
  using reference = typename basic_iterator_traits<T>::reference;

public:
  explicit basic_iterator() = default;

  explicit basic_iterator(pointer ptr):ptr_(ptr){}

  T& operator*() const { return *ptr_; }

  basic_iterator& operator++() {
    ++ptr_;
    return *this;
  }
  basic_iterator operator++(int) {
    basic_iterator temp = *this;
    ++ptr_;
    return temp;
  }

  basic_iterator& operator--() {
    --ptr_;
    return *this;
  }
  basic_iterator operator--(int) {
    basic_iterator temp = *this;
    --ptr_;
    return temp;
  }

  // distance
  difference_type operator-(const basic_iterator other) {
    return ptr_ - other.ptr_;
  }

  basic_iterator& operator+=(difference_type n) {
    ptr_ += n;
    return *this;
  }

  basic_iterator& operator-=(difference_type n) {
    ptr_ -= n;
    return *this;
  }

  basic_iterator operator+(difference_type n) {
    return basic_iterator(ptr_ + n);
  }

  basic_iterator operator-(difference_type n) {
    return basic_iterator(ptr_ - n);
  }

  bool operator==(const basic_iterator&
     other) { return ptr_ == other.ptr_; }

  bool operator!=(const basic_iterator& other) { return ptr_ != other.ptr_; }

  bool operator<(const basic_iterator& other) { return ptr_ < other.ptr_; }

  bool operator>(const basic_iterator& other) { return ptr_ > other.ptr_; }

private:
  T* ptr_;
};

template <typename T>
using Iterator = basic_iterator<T>;

}  // namespace ecs::iterator
