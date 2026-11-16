#pragma once

#include <cstddef>

using MallocFunc = void* (*)(size_t);
using CallocFunc = void* (*)(size_t, size_t);
using ReallocFunc = void* (*)(void*, size_t);
using FreeFunc = void (*)(void*);
using PosixMemalignFunc = int (*)(void**, size_t, size_t);

// The real allocator functions the interposers forward to (per-platform TU),
// looked up the first time each interposer runs.
MallocFunc findMalloc();
CallocFunc findCalloc();
ReallocFunc findRealloc();
FreeFunc findFree();
PosixMemalignFunc findPosixMemalign();
