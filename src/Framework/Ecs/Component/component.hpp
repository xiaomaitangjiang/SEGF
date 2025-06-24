#pragma once

#include<type_traits>
#include"../util.hpp"

namespace ecs::component::basic_component
{

using basic_type::uint32;
using basic_type::uint64;

template<typename T>  
struct basic_component_traits
{
    static_assert(std::is_same_v<std::decay_t<T>, T>, "Unsupported type");

    

};




}
