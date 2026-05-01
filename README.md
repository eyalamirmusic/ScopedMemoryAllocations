# ScopedMemoryAllocations

A tiny C++20 library for catching unwanted heap allocations on real-time threads (audio callbacks, render loops, etc.).

```cpp
#include <ScopedMemoryAllocations/Allocations.h>

void audioCallback(float* out, int frames)
{
    // bans allocs until end of scope
    auto noAllocs = EA::Allocations::ScopedSetter();  

    // any malloc / new in here trips the violation hook
    process(out, frames);
}
```

The library hooks `malloc` / `calloc` / `realloc` / `free` and the global `new` / `delete` operators, so violations are caught **anywhere on the thread** — including inside third-party libraries you don't control. The default hook asserts; install your own to log, record, or trap.

The ban is `thread_local`, so flagging the audio thread doesn't affect the message thread.

Supported on Linux and macOS. On Windows the library compiles to a no-op so the same source builds cross-platform (see [Disabling interposition](#disabling-interposition)).

## Consume with FetchContent

```cmake
# flip to OFF for a no-op build (e.g. release).
# See "Disabling interposition" below for the rationale.
set(SCOPED_ALLOCATIONS_TEST ON)

include(FetchContent)
FetchContent_Declare(
        ScopedMemoryAllocations
        GIT_REPOSITORY https://github.com/eyalamirmusic/ScopedMemoryAllocations.git
        GIT_TAG main
)
FetchContent_MakeAvailable(ScopedMemoryAllocations)

target_link_libraries(MyTarget PRIVATE AllocationsChecker)
```

`AllocationsChecker` is an `INTERFACE` target — its sources are compiled directly into your target so symbol interposition works reliably. Examples and tests only build when this is the top-level project, so `FetchContent` consumers don't drag in extra binaries.

## Manual control

If RAII doesn't fit your call structure:

```cpp
EA::Allocations::setAllowedToAllocate(false);
// ... critical section ...
EA::Allocations::setAllowedToAllocate(true);
```

## Custom violation hooks

The default behaviour on a banned allocation is `assert(false)` (and so compiles out under `NDEBUG`). For tests, logging, or release-mode enforcement, install your own:

```cpp
EA::Allocations::setViolationHandler([] {
    std::fprintf(stderr, "disallowed allocation!\n");
});

EA::Allocations::setViolationHandler({});  // empty restores the default
```

The handler runs with allocation **temporarily re-enabled** on the current thread, so it can freely allocate (logging, formatting, capturing a stack trace) without recursing into itself.

This is what makes the library testable: install a handler that bumps a counter, run the code under test, assert the counter went up. See `Tests/Tests.cpp` for examples.

## Disabling interposition

Interposition is controlled by the `SCOPED_ALLOCATIONS_TEST` CMake option shown above. It becomes a no-op in two cases:

- **Automatically on Windows**, since `dlsym(RTLD_NEXT, ...)` isn't available there — the option is forced `OFF`.
- **Manually**, by setting `SCOPED_ALLOCATIONS_TEST=OFF`.

In both cases the API still compiles and links — calls just don't intercept anything, so cross-platform code needs no `#ifdef` chains.

Treat this as a global, project-wide switch rather than a per-target one. Interposing every `malloc` / `free` / `new` / `delete` adds a `dlsym` lookup and a thread-local check to every allocation in the process, which is significant overhead — you usually want it on for a focused debug/CI profile and off everywhere else, not toggled per consumer.

## Building and running the tests

```sh
cmake -G Ninja -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

Tests use [NanoTest](https://github.com/eyalamirmusic/NanoTest), pulled in via `FetchContent`. Each `nano::test(...)` is its own CTest entry — run one with `ctest --test-dir build -R "name"`.

## License

MIT — see [LICENSE.txt](LICENSE.txt).
