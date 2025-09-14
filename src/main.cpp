#include "Base/ecs_basic_type.hpp"
#include "Framework/Ecs/View/View.hpp"
#include <iostream>

#include <vector>


#undef main

int main()
{  std::vector<int> temp{1,3,5,7,9,11};
    ecs::view::basic_view<std::vector,int> cont(temp);
    for (auto& elem : cont) {
      std::cout<<elem<<std::endl;
    }

}
