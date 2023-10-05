#include "Allocations.h"

#include <cstdlib>
#include <dlfcn.h>
#include <cassert>
#include <utility>

template <typename FuncType>
void initFunc(FuncType& func, const char* funcName)
{
    if (func == nullptr)
    {
        func = (FuncType) dlsym(RTLD_NEXT, funcName);

        if (func == nullptr)
            exit(EXIT_FAILURE);
    }
}

template <typename FuncType, typename... Args>
auto callMem(FuncType& func, const char* name, Args&&... args)
{
    initFunc(func, name);
    assert(EA::Allocations::isAllowedToAllocate());
    return func(std::forward<Args>(args)...);
}

extern "C"
{
    void* (*originalCalloc)(size_t, size_t) = nullptr;
    void* (*originalMalloc)(size_t) = nullptr;
    void* (*originalRealloc)(void*, size_t) = nullptr;
    void (*originalFree)(void*) = nullptr;

    void* malloc(size_t size)
    {
        return callMem(originalMalloc, "malloc", size);
    }

    void* calloc(size_t size, size_t count)
    {
        return callMem(originalCalloc, "calloc", size, count);
    }

    void* realloc(void* ptr, size_t size)
    {
        return callMem(originalRealloc, "realloc", ptr, size);
    }

    void free(void* ptr)
    {
        callMem(originalFree, "free", ptr);
    }
}
