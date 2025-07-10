#pragma once
#include"../util.hpp"
#include "Base/ecs_basic_type.hpp"
#include<type_traits>

namespace ecs::entity::basic_entity
{
using basic_type::uint32;
using basic_type::uint64;


template<typename,typename =void>
struct entity_traits;


template<typename T>
struct entity_traits<T,std::enable_if_t<std::is_enum_v<T>>>
: entity_traits<std::underlying_type_t<T>>
{};

template<typename T>
struct entity_traits<T,std::enable_if_t<std::is_class_v<T>>>
: entity_traits<typename T::entity_type>
{};


template<>
struct entity_traits<uint32>
{
    using value_type=uint32;

    using entity_type=uint32;

    
};


template<>
struct entity_traits<uint64>
{
    using value_type=uint64;

    using entity_type=uint64;
};

}  // namespace ecs::entity::basic_entity


namespace ecs::entity
{
    using basic_entity::entity_traits;
    
    
    /// @brief 
    /// @tparam Basic entity_traits
    template<typename Basic>
    struct basic_entity_traits
    {
        public:
        
        using value_type=typename Basic::value_type;

        using entity_type=typename Basic::entity_type;

        static constexpr entity_type to_integral(const value_type value)
        {return static_cast<entity_type>(value);}

        
    };


    
    template<typename T>
    struct entity :basic_entity_traits<entity_traits<T>>
    {
        using base_type=basic_entity_traits<entity_traits<T>>;

        //static constexpr size_t page_size= ECS_PAGE_SIZE;
    };
    
}  // namespace ecs::entity

