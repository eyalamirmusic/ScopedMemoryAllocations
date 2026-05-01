#ifdef EA_SCOPED_ALLOCATIONS_TEST

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

void* operator new(std::size_t size, std::align_val_t align)
{
    auto* ptr = static_cast<void*>(nullptr);

    if (posix_memalign(&ptr, static_cast<std::size_t>(align), size) != 0)
        throw std::bad_alloc();

    return ptr;
}

void* operator new[](std::size_t size, std::align_val_t align)
{
    auto* ptr = static_cast<void*>(nullptr);

    if (posix_memalign(&ptr, static_cast<std::size_t>(align), size) != 0)
        throw std::bad_alloc();

    return ptr;
}

void operator delete(void* ptr) noexcept
{ free(ptr); }

void operator delete[](void* ptr) noexcept
{ free(ptr); }

void operator delete(void* ptr, std::align_val_t) noexcept
{ free(ptr); }

void operator delete[](void* ptr, std::align_val_t) noexcept
{ free(ptr); }

void operator delete(void* ptr, std::size_t, std::align_val_t) noexcept
{ free(ptr); }

void operator delete[](void* ptr, std::size_t, std::align_val_t) noexcept
{ free(ptr); }

#endif // EA_SCOPED_ALLOCATIONS_TEST
