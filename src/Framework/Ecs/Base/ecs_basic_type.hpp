#pragma once
#include <cstdint>

namespace ecs::basic_type {

using uint8 = std::uint8_t;
using uint16 = std::uint16_t;
using uint32 = std::uint32_t;
using uint64 = std::uint64_t;

/// @brief most basic type of entity
using default_type = uint32;

enum class entity : uint32 {};

// @brief basic type of TypeID
using TypeID = uint64;

}  // namespace ecs::basic_type
