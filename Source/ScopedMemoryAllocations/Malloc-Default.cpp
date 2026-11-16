#include "Malloc.h"

#include <cstdlib>
#include <dlfcn.h>

template <typename FuncType>
FuncType resolve(const char* funcName)
{
    auto func = (FuncType) dlsym(RTLD_NEXT, funcName);

    if (func == nullptr)
        exit(EXIT_FAILURE);

    return func;
}

MallocFunc findMalloc()
{ return resolve<MallocFunc>("malloc"); }

CallocFunc findCalloc()
{ return resolve<CallocFunc>("calloc"); }

ReallocFunc findRealloc()
{ return resolve<ReallocFunc>("realloc"); }

FreeFunc findFree()
{ return resolve<FreeFunc>("free"); }

PosixMemalignFunc findPosixMemalign()
{ return resolve<PosixMemalignFunc>("posix_memalign"); }
