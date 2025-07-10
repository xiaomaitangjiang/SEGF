#pragma once


#include <atomic>


#include "Base/ecs_basic_type.hpp"

namespace ecs::TypeIndex {

using basic_type::TypeID;


inline std::atomic<TypeID>& global_type_counter() noexcept {
  static std::atomic<TypeID> counter(0);
  return counter;
}


template <typename T>
class TypeIndex {
  static TypeID get_id() noexcept {
    static TypeID id = global_type_counter()++;
    return id;
  }

public:
  static TypeID value() noexcept {
    static const TypeID id = get_id();
    return id;
  }

};

}  // namespace ecs::TypeIndex
