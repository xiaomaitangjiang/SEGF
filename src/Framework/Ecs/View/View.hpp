#pragma once

#include <float.h>
#include <type_traits>

#include "../Base/iterator.hpp"
#include "util.hpp"

namespace ecs::view {

template <typename T>
class basic_view_iterator : public iterator::basic_iterator<T>
{
public:
  using basic_iterator = iterator::basic_iterator<T>;

  using iterator_category = typename basic_iterator::iterator_category;
  using value_type = typename basic_iterator::value_type;
  using difference_type = typename basic_iterator::difference_type;
  using pointer = typename basic_iterator::pointer;
  using reference = typename basic_iterator::reference;

  using Storage=typename basic_iterator::Storage;

public:
  explicit basic_view_iterator() = default;

  explicit basic_view_iterator(Storage ptr) : basic_iterator(ptr) {}

  template <typename OtherIter,
            std::enable_if_t<!std::is_same_v<OtherIter, basic_iterator> &&
                                 ecs::util::is_iterator_v<OtherIter>,
                             int> = 0>
  basic_view_iterator(const OtherIter& other) : basic_iterator(other)
  {
  }
};

/**
 * @brief 轻量级容器视图类，提供对容器的非持有引用
 *
 * 此类为容器类型提供只读视图，要求目标容器必须提供标准的 STL 风格迭代器。
 * 视图不拥有容器数据，生命周期必须短于被引用的容器。
 *
 * @tparam container  容器模板类型（如 std::vector, std::list）
 * @tparam elements   容器模板所需的元素类型参数包（如 int 对于
 * std::vector<int>）
 *
 * @note 使用 static_assert 确保容器满足迭代器要求：
 *       - 必须提供有效的 `iterator` 类型成员
 *       - 迭代器必须符合 STL 迭代器基本规范
 *
 * @note 此类只能提供对单个容器的非持有引用
 *
 */
template <template <typename... Args> typename container, typename... elements>
class basic_view
{
  using Container_T = std::remove_cv_t<container<elements...>>;

  using ValueType = typename Container_T::value_type;

  using Iterator = basic_view_iterator<ValueType>;

  using Iterator_Wapper=basic_view_iterator<typename Container_T::iterator>;

  static_assert(util::is_valid_iterator_v<typename Container_T::iterator>,
                "invalid container");

public:
  /**
   * @brief 通过现有容器构造视图
   * @param reg 被引用的目标容器
   * @warning 视图不延长容器生命周期，调用者需保证容器存活时间长于视图
   */
  explicit basic_view(Container_T& reg) : reg_reference(reg) {}

  /**
   * @brief 在容器中查找指定元素
   *
   * 使用线性搜索算法查找元素，时间复杂度为 O(n)。查找过程中使用元素的
   * `operator==` 进行相等性比较。
   *
   * @param value 要查找的目标元素值（常量引用传递）
   * @return 指向第一个匹配元素的迭代器，若未找到则返回 end() 迭代器
   *
   * @note 要求容器元素类型支持相等比较（equality comparable）
   * @see end()
   */
  typename Container_T::iterator find(const ValueType& value) const noexcept
  {
    return std::find(reg_reference.begin(), reg_reference.end(), value);
  }

  /**
   * @brief 在容器中查找指定元素
   *
   * 使用线性搜索算法查找元素，时间复杂度为 O(n)。查找过程中使用元素的
   * `operator==` 进行相等性比较。
   *
   * @param pred 要查找的目标元素
   * @return 指向第一个匹配元素的迭代器，若未找到则返回 end() 迭代器
   *
   * @note 要求容器元素类型支持相等比较（equality comparable）
   * @see end()
   */
  template <typename Predicate>
  typename Container_T::iterator find_if(Predicate pred) const
  {
    return std::find_if(reg_reference.begin(), reg_reference.end(), pred);
  }

  /**
   * @brief 获取容器头部迭代器
   *
   * 与标准 STL 容器一致，返回指向容器头部的迭代器。
   */
  Iterator_Wapper begin() const noexcept {
    return Iterator_Wapper(reg_reference.begin());
  }

  /**
   * @brief 获取容器尾后迭代器
   *
   * 与标准 STL 容器一致，返回指向容器末尾的迭代器，常用于表示查找失败。
   */
  Iterator_Wapper end() const noexcept {
    return Iterator_Wapper(reg_reference.end());
  }

private:
  Container_T& reg_reference;
};

}  // namespace ecs::view