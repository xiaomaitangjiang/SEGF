#pragma once

#include <cassert>
#include <cstdint>
#include <iterator>
#include <type_traits>

#define assertm(msg, expr) assert(((void)msg, (expr)))

namespace ecs::basic_type {

using uint32 = std::uint32_t;
using uint64 = std::uint64_t;

}  // namespace ecs::basic_type

namespace ecs::util {

// Primary template.
///

template <typename T, typename = void>
struct try_underlying_type;

template <typename T>
struct try_underlying_type<T, std::enable_if_t<!std::is_enum_v<T>, T>> {
  using Type = T;
};

template <typename T>
struct try_underlying_type<T, std::enable_if_t<std::is_enum_v<T>, T>> {
  using Type = std::underlying_type_t<T>;
};

/// @brief Metaprogramming helper types.
/// @brief Define a member typedef `type` of T
/// @tparam T
template <typename T>
using try_underlying_type_t = typename try_underlying_type<T>::Type;

template <typename Iter, typename = void>
struct is_iterator : std::false_type {};

template <typename Iter>
struct is_iterator<
    Iter, std::void_t<typename std::iterator_traits<Iter>::value_type,
                      typename std::iterator_traits<Iter>::iterator_category,
                      typename std::iterator_traits<Iter>::difference_type,
                      typename std::iterator_traits<Iter>::pointer,
                      typename std::iterator_traits<Iter>::reference>>
    : std::true_type {};

/// @brief Metaprogramming helper types.
/// @brief
/// @tparam Iter
template <typename Iter>
inline constexpr bool is_iterator_v = is_iterator<Iter>::value;

template <typename Iter, typename = void>
struct is_valid_iterator : std::false_type {};

template <typename Iter>
struct is_valid_iterator<
    Iter, std::void_t<
              std::enable_if_t<is_iterator_v<Iter>, Iter>,
              std::is_convertible<
                  typename std::iterator_traits<Iter>::iterator_category*,
                  std::input_iterator_tag*>,
              decltype(*std::declval<Iter>()), decltype(++std::declval<Iter>()),
              decltype(std::declval<Iter>() != std::declval<Iter>())>>
    : std::true_type {};

/// @brief Metaprogramming helper types.
/// @brief
/// @tparam Iter
template <typename Iter>
inline constexpr bool is_valid_iterator_v = is_valid_iterator<Iter>::value;

template <typename Iter>
using is_valid_iterator_t = std::enable_if_t<is_valid_iterator_v<Iter>>;

template <typename Iter_Form_, typename Iter_To_, typename = void>
struct is_equal_iterator : std::false_type {};

template <typename Iter_Form_, typename Iter_To_>
struct is_equal_iterator<
    Iter_Form_, Iter_To_,
    std::void_t<std::enable_if_t<is_valid_iterator_v<Iter_Form_>>,
                std::enable_if_t<is_valid_iterator_v<Iter_To_>>,
                std::enable_if_t<std::is_convertible_v<Iter_Form_, Iter_To_>>>>
    : std::true_type {};

template <typename Iter_Form, typename Iter_To>
inline constexpr bool is_equal_iterator_v =
    is_equal_iterator<Iter_Form, Iter_To>::value;

template <typename Iter_Form, typename Iter_To>
using is_equal_iterator_t =
    std::enable_if_t<is_equal_iterator_v<Iter_Form, Iter_To>>;

    template<typename Type_1_,typename Type_2_>
    using is_same_t =std::enable_if_t<std::is_same_v<Type_1_, Type_2_>>;


}  // namespace ecs::util
