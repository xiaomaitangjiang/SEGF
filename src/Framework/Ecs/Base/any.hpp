#pragma once

//warning:this facility wasn't finished,do not use it

#include <cstdint>
#include <memory>
#include <type_traits>

#include <utility>

#include "Base/ecs_basic_type.hpp"
#include "type_index/type_index.hpp"

namespace ecs::any {
enum class Mods : std::uint8_t { null, Runtime, Compiletime };

template <typename T, typename = void>
struct any_strategy {};

template <typename T>
struct any_strategy<T, std::enable_if_t<std::is_void_v<T>>> {
  Mods strategy = Mods::Runtime;
};

template <typename T>
struct any_strategy<T, std::enable_if_t<!std::is_void_v<T>>> {
  Mods strategy = Mods::Compiletime;
};

template <typename T>
class Compiletime_storage {
  
};

using basic_type::TypeID;

class any_storage {
public:
  virtual ~any_storage() = default;
  virtual TypeID Type();
};

template <typename T>
class basic_storage : any_storage {
public:
  explicit basic_storage() = default;
  explicit basic_storage(const T& value) : data(value) {}
  explicit basic_storage(T&& value) : data(std::move(value)) {}

  TypeID Type() override {
    static TypeID ID = TypeIndex::TypeIndex<T>::value();
    return ID;
  }

private:
  T data;
};

template <typename T,std::enable_if_t<std::is_void_v<T>>>
class Any {
public:
  Any() = default;
  Any(const Any& other) {}

  ~Any() = default;

  template <typename ValueType, typename... Args>
  std::decay_t<ValueType>& emplace(Args... args) {
    data.reset(
        new basic_storage<std::decay_t<ValueType>>(std::move((args, ...))));
  }

  void reset() { data.reset(); }

  TypeID Type() { return data->Type(); }

  // warning: this function will get failled at c++20
  [[nodiscard]] bool has_value() const noexcept { return data != nullptr; }
private:
  union Storage {
    std::unique_ptr<any_storage> runtime_data;
    Compiletime_storage<T> compiletime_data;
  };
private:
  Mods _any_mod = Mods::Runtime;
  Storage data;
};
}  // namespace ecs::any
