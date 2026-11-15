#ifdef EA_SCOPED_ALLOCATIONS_TEST

#include "Allocations.h"

#include <cstdlib>
#include <dlfcn.h>
#include <utility>

using FreeFunc = void (*)(void*);

// What free uses while dlsym resolves the real one (per-platform TU).
FreeFunc getBootstrapFree();

template <typename FuncType>
FuncType resolve(const char* funcName)
{
    auto func = (FuncType) dlsym(RTLD_NEXT, funcName);

    if (func == nullptr)
        exit(EXIT_FAILURE);

    return func;
}

template <typename FuncType>
void initFunc(FuncType& func, const char* funcName)
{
    if (func == nullptr)
        func = resolve<FuncType>(funcName);
}

template <typename FuncType, typename... Args>
auto callMem(FuncType& func, const char* name, Args&&... args)
{
    initFunc(func, name);
    if (!EA::Allocations::isAllowedToAllocate())
        EA::Allocations::onAllocationViolation();
    return func(std::forward<Args>(args)...);
}

extern "C"
{
    void* (*originalCalloc)(size_t, size_t) = nullptr;
    void* (*originalMalloc)(size_t) = nullptr;
    void* (*originalRealloc)(void*, size_t) = nullptr;
    FreeFunc originalFree = nullptr;
    int (*originalPosixMemalign)(void**, size_t, size_t) = nullptr;

    void* malloc(size_t size)
    { return callMem(originalMalloc, "malloc", size); }

    void* calloc(size_t size, size_t count)
    { return callMem(originalCalloc, "calloc", size, count); }

    void* realloc(void* ptr, size_t size)
    { return callMem(originalRealloc, "realloc", ptr, size); }

    void free(void* ptr)
    {
        // glibc's dlsym can free a pending dlerror string mid-lookup.
        if (originalFree == nullptr)
        {
            originalFree = getBootstrapFree();
            originalFree = resolve<FreeFunc>("free");
        }

        callMem(originalFree, "free", ptr);
    }

    int posix_memalign(void** memptr, size_t alignment, size_t size)
    {
        return callMem(
            originalPosixMemalign, "posix_memalign", memptr, alignment, size);
    }
}

#endif // EA_SCOPED_ALLOCATIONS_TEST
