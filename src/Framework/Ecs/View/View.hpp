#pragma once

#include <cstddef>

#include "../Base/iterator.hpp"
#include "../Registry/registry.hpp"


namespace ecs::view {

using ecs::registry::basic_registry::registry;

template <typename T>
class basic_view_iterator : public ecs::iterator::basic_iterator<T> {};

template <typename Entity, typename... Components>
class basic_view {
public:
  basic_view() = default;

  basic_view(registry<Entity>& reg) : reg_reference(reg) {}

  void each(void (*fn)(Entity, Components...)) 
  {
    
  }

private:
  registry<Entity>& reg_reference;
};

}  // namespace ecs::view