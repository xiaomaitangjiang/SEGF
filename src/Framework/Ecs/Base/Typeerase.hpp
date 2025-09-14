#pragma once

#include <cstddef>
#include <functional>
#include <type_traits>
#include <utility>

//notice: 目前

namespace ecs::Typeerase {
/**
 * @class TypeErase
 * @brief 类型擦除容器，用于存储和管理任意类型的对象
 *
 * 该类提供类型安全的存储功能，并支持对存储对象执行可调用操作。
 * 采用小缓冲区优化（SBO）技术，对于小于BufferSize的对象避免堆分配。
 * 存储的对象可以通过提供的函数对象进行访问和操作。
 */
class TypeErase
{
  static constexpr size_t BufferSize = 64;  ///< 内部存储缓冲区大小
  using Buffer = std::byte[BufferSize];     ///< 内部缓冲区类型别名

  void* data_;                               ///< 指向存储对象的指针
  alignas(std::max_align_t) Buffer buffer_;  ///< 用于SBO的内部存储缓冲区
  void (*deleter_)(void*);                   ///< 对象析构函数指针
  std::function<void(void*)> executor_;      ///< 执行操作的函数对象
  bool using_buffer_;                        ///< 标记是否使用内部缓冲区

public:
  /**
   * @brief 使用对象和执行函数构造TypeErase实例
   * @tparam T 要存储的对象类型
   * @tparam Func 执行函数的类型（必须是函数指针）
   * @param value 要存储和管理的对象
   * @param func 用于对存储对象执行操作的函数指针
   * @throws std::bad_alloc 当不使用内部缓冲区时，堆分配失败抛出
   *
   * @note Func必须衰减为函数指针类型。函数应接受T类型（或const
   * T&）参数并返回void。
   *       使用小缓冲区优化时，对象直接在内部缓冲区构造；否则在堆上分配。
   */
  template <typename T, typename Func>
  explicit TypeErase(T value, Func func)
  {
    static_assert(std::is_pointer_v<std::decay_t<Func>>,
                  "The decay type of Func isn't a pointer");
    static_assert(std::is_function_v<std::remove_pointer_t<std::decay_t<Func>>>,
                  "func is not a function");

    using_buffer_ =
        (sizeof(T) <= BufferSize) && (alignof(T) <= alignof(decltype(buffer_)));

    if (using_buffer_) {
      // 构造到 buffer_ 中
      new (&buffer_) T(std::move(value));
      data_ = &buffer_;
      deleter_ = [](void* ptr) { static_cast<T*>(ptr)->~T(); };
    }
    else {
      // 在堆上分配
      data_ = new T(std::move(value));
      deleter_ = [](void* ptr) { delete static_cast<T*>(ptr); };
    }
    executor_ = [func](void* ptr) { func(*static_cast<T*>(ptr)); };
  }

  TypeErase(const TypeErase&) = delete;

  TypeErase& operator=(const TypeErase&) = delete;
  /**
   * @brief 析构函数，清理存储的对象
   * @details 根据对象的存储位置调用相应的删除器进行清理
   */
  ~TypeErase() { deleter_(data_); }
  /**
   * @brief 对托管对象执行存储的函数操作
   * @throws 可能传播存储函数对象抛出的异常
   */
  void execute() { executor_(data_); }
};
}  // namespace ecs::Typeerase