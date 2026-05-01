# ScopedMemoryAllocations

A tiny C++20 library for catching unwanted heap allocations on real-time threads (audio callbacks, render loops, ISRs that share state with userspace, etc.).

Mark a thread or scope as "no allocations allowed", and any subsequent `malloc` / `calloc` / `realloc` / `free` / `new` / `delete` will route through a hook of your choice. The default hook asserts; for tests you can install your own callback.

The flag is `thread_local`, so banning allocations on the audio thread does not affect the message thread.

Supported platforms: Linux and macOS (uses `dlsym(RTLD_NEXT, ...)`). Compiles to a no-op on Windows so the same code can build cross-platform; see [Disabling interposition](#disabling-interposition) below.

## Consume with FetchContent

```cmake
include(FetchContent)
FetchContent_Declare(
        ScopedMemoryAllocations
        GIT_REPOSITORY https://github.com/eyalamirmusic/ScopedMemoryAllocations.git
        GIT_TAG main
)
FetchContent_MakeAvailable(ScopedMemoryAllocations)

target_link_libraries(MyTarget PRIVATE AllocationsChecker)
```

`AllocationsChecker` is an `INTERFACE` target. Its source files (the `malloc`/`new` overrides) are compiled directly into your target — this is required for symbol interposition to work reliably with the linker.

The library only adds its own examples and tests when it is the top-level CMake project, so consuming it via `FetchContent` won't drag in NanoTest or build extra binaries.

## Basic usage

```cpp
#include <ScopedMemoryAllocations/Allocations.h>

void audioCallback(float* out, int frames)
{
    EA::Allocations::ScopedSetter noAllocs;  // bans allocs until end of scope

    // any malloc / new in here trips the violation hook
    process(out, frames);
}
```

Manual control is also available if RAII doesn't fit your call structure:

```cpp
EA::Allocations::setAllowedToAllocate(false);
// ... critical section ...
EA::Allocations::setAllowedToAllocate(true);
```

The flag lives in `thread_local` storage, so each thread carries its own state. Banning on one thread never affects another.

## Custom violation hooks

The default behaviour on a banned allocation is `assert(false)`. For tests, logging, or release-mode enforcement, install your own handler:

```cpp
#include <ScopedMemoryAllocations/Allocations.h>

EA::Allocations::setViolationHandler([] {
    // record, log, breakpoint, throw — whatever you need
    std::fprintf(stderr, "disallowed allocation!\n");
});
```

Pass an empty `std::function` to restore the default:

```cpp
EA::Allocations::setViolationHandler({});
```

The handler is invoked with allocation **temporarily re-enabled** on the current thread, so it is free to allocate (logging, formatting, building a stack trace) without recursing into itself. The previous flag value is restored when the handler returns.

This is what makes the library testable: install a handler that bumps a counter, run the code under test, assert the counter went up. See `Tests/Tests.cpp` for examples.

## Disabling interposition

Two ways to turn the library into a no-op:

- **Automatically on Windows.** The `dlsym(RTLD_NEXT, ...)` mechanism doesn't exist on Windows, so the build defines `EA_SCOPED_ALLOCATIONS_DISABLE` for you. The API (`ScopedSetter`, `setViolationHandler`, etc.) still compiles and links — calls just don't intercept anything. This lets cross-platform code use the same headers without `#ifdef` chains.

- **Manually, on supported platforms.** Pass `-DScoped_Allocations_Disable=ON` at configure time:

  ```sh
  cmake -DScoped_Allocations_Disable=ON ...
  ```

  Same effect: `EA_SCOPED_ALLOCATIONS_DISABLE` is defined on the `AllocationsChecker` interface, the `malloc` / `new` overrides become empty, and `dl` is no longer linked.

Either way, source code that uses `EA::Allocations::ScopedSetter` etc. continues to compile unchanged — it just doesn't catch anything at runtime.

## Building and running the tests

```sh
cmake -G Ninja -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

Tests use [NanoTest](https://github.com/eyalamirmusic/NanoTest), pulled in via `FetchContent`. Each `nano::test(...)` is registered as its own CTest entry, so a single test can be run with:

```sh
./build/Tests/ScopedAllocationsTests --test "name"
# or
ctest --test-dir build -R "name"
```

The example program (`./build/Examples/Example`) intentionally aborts on its "audio thread" to demonstrate the default assert behaviour.

## License

MIT — see [LICENSE.txt](LICENSE.txt).
