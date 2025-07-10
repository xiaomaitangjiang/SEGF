#pragma once

#include "../util.hpp"
#include "Base/ecs_basic_type.hpp"

namespace ecs::basic_value {
inline static constexpr std::size_t ECS_PAGE_SIZE = 4096;

inline static constexpr std::size_t ECS_COMPONENT_PAGE_SIZE = 1024;

inline static constexpr basic_type::uint16 MaxComponentTypes = 128;

}  // namespace ecs::basic_value
