#ifdef EA_SCOPED_ALLOCATIONS_TEST

#include "Allocations.h"
#include "Malloc.h"

#include <cstdlib>
#include <utility>

template <typename FuncType, typename FindFunc>
void initFunc(FuncType& func, FindFunc find)
{
    if (func == nullptr)
        func = find();
}

template <typename FuncType, typename FindFunc, typename... Args>
auto callMem(FuncType& func, FindFunc find, Args&&... args)
{
    initFunc(func, find);
    if (!EA::Allocations::isAllowedToAllocate())
        EA::Allocations::onAllocationViolation();
    return func(std::forward<Args>(args)...);
}

extern "C"
{
    CallocFunc originalCalloc = nullptr;
    MallocFunc originalMalloc = nullptr;
    ReallocFunc originalRealloc = nullptr;
    FreeFunc originalFree = nullptr;
    PosixMemalignFunc originalPosixMemalign = nullptr;

    void* malloc(size_t size)
    { return callMem(originalMalloc, findMalloc, size); }

    void* calloc(size_t size, size_t count)
    { return callMem(originalCalloc, findCalloc, size, count); }

    void* realloc(void* ptr, size_t size)
    { return callMem(originalRealloc, findRealloc, ptr, size); }

    void free(void* ptr)
    { callMem(originalFree, findFree, ptr); }

    int posix_memalign(void** memptr, size_t alignment, size_t size)
    {
        return callMem(
            originalPosixMemalign, findPosixMemalign, memptr, alignment, size);
    }
}

#endif // EA_SCOPED_ALLOCATIONS_TEST
