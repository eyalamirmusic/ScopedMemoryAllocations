#ifndef EA_SCOPED_ALLOCATIONS_DISABLE

#include <new>
#include <cstdlib>

void* operator new(std::size_t size)
{
    auto* ptr = malloc(size);

    if (ptr == nullptr)
        throw std::bad_alloc();

    return ptr;
}

void* operator new[](std::size_t size)
{
    auto* ptr = malloc(size);

    if (ptr == nullptr)
        throw std::bad_alloc();

    return ptr;
}

void operator delete(void* ptr) noexcept
{
    free(ptr);
}

void operator delete[](void* ptr) noexcept
{
    free(ptr);
}

#endif // !EA_SCOPED_ALLOCATIONS_DISABLE
