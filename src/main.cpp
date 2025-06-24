#include <iostream>
#include <iterator>
#include <mutex>
#include "Framework/Ecs/Registry/rejistry.hpp"

struct test{
    int a;
};

int main()
{
    test xiaoming{10};
    ecs::registry::basic_registry::componentstorage<test,int> ste;
    ste.add(1,xiaoming);
    ste.add(2, xiaoming);

    ecs::registry::basic_registry::componentstorage<test,int> ste2;
    ste2.add_batch(ste.begin(),ste.end() );

    return 0;
}
