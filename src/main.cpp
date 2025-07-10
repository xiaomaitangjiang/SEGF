#include "Base/ecs_basic_type.hpp"
#include "Framework/Ecs/Registry/registry.hpp"

#undef main

int main()
{
    ecs::registry::basic_registry::registry<ecs::basic_type::uint32> a;
    return 0;
}
