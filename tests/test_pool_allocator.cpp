#include "rurouni/core/memory/PoolAllocator.hpp"
#include <catch2/catch.hpp>

class Person
{
public:
    Person(int age)
        : m_age(age) {}
private:
    int m_age;
};

TEST_CASE( "pool memory allocations", "[allocate]" ) {

    Rurouni::PoolAllocator<Person> m_personPoolAlloc;

    Person* p1 = m_personPoolAlloc.allocate(12);
    Person* p2 = m_personPoolAlloc.allocate(31);
    Person* p3 = m_personPoolAlloc.allocate(55);

    m_personPoolAlloc.deallocate(p1);
    m_personPoolAlloc.deallocate(p2);
    m_personPoolAlloc.deallocate(p3);

}
