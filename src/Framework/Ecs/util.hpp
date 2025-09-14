#pragma once

#include <cassert>
#include <iterator>
#include <limits>
#include <type_traits>
#include <utility>

#define assertm(msg, expr) assert(((void)msg, (expr)))

namespace ecs::util {

// Primary template.
///

/**
 * @brief 基础类型提取模板
 * @details 用于安全地获取枚举类型的底层基础类型，对于非枚举类型则返回自身类型
 * @tparam T 待提取的类型
 * @tparam 辅助的SFINAE参数，默认void
 */
template <typename T, typename = void>
struct try_underlying_type;

/**
 * @brief 非枚举类型的特化版本
 * @tparam T 非枚举类型
 */
template <typename T>
struct try_underlying_type<T, std::enable_if_t<!std::is_enum_v<T>, T>>
{
  using Type = T;  ///< 直接使用原类型
};

/**
 * @brief 枚举类型的特化版本
 * @tparam T 枚举类型
 */
template <typename T>
struct try_underlying_type<T, std::enable_if_t<std::is_enum_v<T>, T>>
{
  using Type = std::underlying_type_t<T>;  ///< 使用底层类型
};

/**
 * @brief 类型提取的便捷别名
 * @tparam T 待提取的类型
 */
template <typename T>
using try_underlying_type_t = typename try_underlying_type<T>::Type;

/**
 * @brief 迭代器类型特征检查模板
 * @tparam Iter 待检查的类型
 * @tparam SFINAE辅助参数
 */
template <typename Iter, typename = void>
struct is_iterator : std::false_type
{
};

/**
 * @brief 满足迭代器特征的特化版本
 * @tparam Iter 具有完整迭代器特征的类型
 */
template <typename Iter>
struct is_iterator<
    Iter, std::void_t<typename std::iterator_traits<Iter>::value_type,
                      typename std::iterator_traits<Iter>::iterator_category,
                      typename std::iterator_traits<Iter>::difference_type,
                      typename std::iterator_traits<Iter>::pointer,
                      typename std::iterator_traits<Iter>::reference>>
    : std::true_type
{
};

/**
 * @brief 迭代器检查的编译时常量
 * @tparam Iter 待检查的类型
 */
template <typename Iter>
inline constexpr bool is_iterator_v = is_iterator<Iter>::value;

/**
 * @brief 迭代器有效性检查模板
 * @tparam Iter 待检查的迭代器类型
 * @tparam SFINAE辅助参数
 */
template <typename Iter, typename = void>
struct is_valid_iterator : std::false_type
{
};

/**
 * @brief 有效迭代器的特化版本
 * @details 检查迭代器是否支持基本操作：解引用、递增、比较
 * @tparam Iter 满足操作要求的迭代器类型
 */
template <typename Iter>
struct is_valid_iterator<
    Iter, std::void_t<
              std::enable_if_t<is_iterator_v<Iter>, Iter>,
              std::is_convertible<
                  typename std::iterator_traits<Iter>::iterator_category*,
                  std::input_iterator_tag*>,
              decltype(*std::declval<Iter>()), decltype(++std::declval<Iter>()),
              decltype(std::declval<Iter>() != std::declval<Iter>())>>
    : std::true_type
{
};

/**
 * @brief 迭代器有效性检查的编译时常量
 * @tparam Iter 待检查的迭代器类型
 */
template <typename Iter>
inline constexpr bool is_valid_iterator_v = is_valid_iterator<Iter>::value;

/**
 * @brief 用于SFINAE的迭代器有效性类型别名
 * @tparam Iter 迭代器类型
 */
template <typename Iter>
using is_valid_iterator_t = std::enable_if_t<is_valid_iterator_v<Iter>>;

/**
 * @brief 迭代器类型等价性检查模板
 * @tparam Iter_Form_ 源迭代器类型
 * @tparam Iter_To_ 目标迭代器类型
 * @tparam SFINAE辅助参数
 */
template <typename Iter_Form_, typename Iter_To_, typename = void>
struct is_equal_iterator : std::false_type
{
};

/**
 * @brief 迭代器类型兼容的特化版本
 * @tparam Iter_Form_ 可转换为目标类型的源迭代器类型
 * @tparam Iter_To_ 目标迭代器类型
 */
template <typename Iter_Form_, typename Iter_To_>
struct is_equal_iterator<
    Iter_Form_, Iter_To_,
    std::void_t<std::enable_if_t<is_valid_iterator_v<Iter_Form_>>,
                std::enable_if_t<is_valid_iterator_v<Iter_To_>>,
                std::enable_if_t<std::is_convertible_v<Iter_Form_, Iter_To_>>>>
    : std::true_type
{
};

/**
 * @brief 迭代器类型等价性检查的编译时常量
 * @tparam Iter_Form 源迭代器类型
 * @tparam Iter_To 目标迭代器类型
 */
template <typename Iter_Form, typename Iter_To>
inline constexpr bool is_equal_iterator_v =
    is_equal_iterator<Iter_Form, Iter_To>::value;

/**
 * @brief 用于SFINAE的迭代器类型等价性类型别名
 * @tparam Iter_Form 源迭代器类型
 * @tparam Iter_To 目标迭代器类型
 */
template <typename Iter_Form, typename Iter_To>
using is_equal_iterator_t =
    std::enable_if_t<is_equal_iterator_v<Iter_Form, Iter_To>>;

/**
 * @brief 类型相同性检查的SFINAE辅助类型
 * @tparam Type_1_ 第一个类型
 * @tparam Type_2_ 第二个类型
 */
template <typename Type_1_, typename Type_2_>
using is_same_t = std::enable_if_t<std::is_same_v<Type_1_, Type_2_>>;

/**
 * @brief 整数类型最大值常量模板
 * @tparam T 整数类型，通过SFINAE限制
 */
template <typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
inline constexpr T max = std::numeric_limits<T>::max();

/**
 * @brief 检查相邻类型对是否相同的辅助函数模板
 *
 * 该函数通过元组和索引序列展开，在编译时检查所有相邻的两个类型是否相同。
 * 使用折叠表达式和std::tuple_element_t来获取每对类型并进行比较。
 *
 * @tparam T 第一个类型
 * @tparam T1 第二个类型
 * @tparam Ts 剩余的类型参数包
 * @tparam U 索引序列的索引值（非类型模板参数包）
 * @param seq 索引序列，用于展开检查每对类型
 * @return bool 所有相邻类型对是否相同
 */
template <typename T, typename T1, typename... Ts, std::size_t... U>
bool check_adjacent_pairs(std::index_sequence<U...>)
{
  using type = std::tuple<T, T1, Ts...>;

  return ((std::is_same_v<std::tuple_element_t<(U * 2), type>,
                          std::tuple_element_t<(U * 2) + 1, type>>) &&
          ...);
}

/**
 * @brief 检查所有相邻的两个类型是否相同的模板结构体
 *
 * 该结构体用于在编译时检查所有相邻的两个模板参数类型是否相同。
 * 要求参数个数为偶数（除了0和1个参数的情况外），通过递归和模板元编程实现。
 *
 * @tparam Ts 要检查的类型列表
 */
template <typename... Ts>
struct all_both_same;

/**
 * @brief 空参数包的特化版本，继承std::true_type
 *
 * 当没有类型参数时，视为空类型与空类型比较，结果相同。
 * 这种情况满足"所有相邻对相同"的条件。
 */
template <>
struct all_both_same<> : std::true_type
{
};

/**
 * @brief 单个类型参数的特化版本，继承std::false_type
 *
 * 单个类型参数被视为与一个空类型进行比较。
 * 由于任何非空类型与空类型都不相同，因此不满足条件。
 */
template <typename T>
struct all_both_same<T> : std::false_type
{
};

/**
 * @brief 两个及以上类型参数的主模板
 *
 * 通过递归检查相邻的两个类型是否相同来确定结果。
 *
 * @tparam T 第一个类型
 * @tparam T1 第二个类型
 * @tparam Ts 剩余的类型参数包
 */
template <typename T, typename T1, typename... Ts>
struct all_both_same<T, T1, Ts...>
{
private:
  /**
   * @brief 检查相邻类型对是否相同的核心方法
   *
   * 使用静态断言确保参数个数为偶数，然后递归检查每对相邻类型。
   *
   * @return constexpr bool 所有相邻类型对是否相同
   */
  static constexpr bool check_pairs()
  {
    static_assert(static_cast<bool>(sizeof...(Ts) % 2),
                  "The number of template parameter should be even");

    if constexpr (sizeof...(Ts) == 0) {
      return std::is_same_v<T, T1>;
    }
    else {
      constexpr std::size_t count = sizeof...(Ts) + 2;
      return check_adjacent_pairs<T, T1, Ts...>(
          std::make_index_sequence<count / 2>{});
    }
  }

public:
  static constexpr bool value = check_pairs();
};

}  // namespace ecs::util
