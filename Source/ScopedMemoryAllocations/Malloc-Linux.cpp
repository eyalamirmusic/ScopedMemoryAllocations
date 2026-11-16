#include "Malloc.h"

#include <cerrno>

// glibc's own entry points into its allocator. Binding to them directly keeps
// dlsym out of the interposers: glibc's dlsym frees a pending dlerror string
// before clearing it, so a lookup from inside free would free it twice.
extern "C"
{
    void* __libc_malloc(size_t);
    void* __libc_calloc(size_t, size_t);
    void* __libc_realloc(void*, size_t);
    void __libc_free(void*);
    void* __libc_memalign(size_t, size_t);
}

namespace
{
// glibc exports no __libc_posix_memalign: this is its posix_memalign over
// __libc_memalign.
int posixMemalign(void** memptr, size_t alignment, size_t size)
{
    const auto isPowerOfTwo = (alignment & (alignment - 1)) == 0;

    if (alignment % sizeof(void*) != 0 || !isPowerOfTwo || alignment == 0)
        return EINVAL;

    auto* ptr = __libc_memalign(alignment, size);

    if (ptr == nullptr)
        return ENOMEM;

    *memptr = ptr;
    return 0;
}
} // namespace

MallocFunc findMalloc()
{ return __libc_malloc; }

CallocFunc findCalloc()
{ return __libc_calloc; }

ReallocFunc findRealloc()
{ return __libc_realloc; }

FreeFunc findFree()
{ return __libc_free; }

PosixMemalignFunc findPosixMemalign()
{ return posixMemalign; }
