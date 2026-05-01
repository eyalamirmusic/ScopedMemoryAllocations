# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build & test

CMake (C++20) project.

```sh
cmake -G Ninja -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
./build/Examples/Example          # intentionally aborts on the "audio thread"
```

Tests live in `Tests/` and use [NanoTest](https://github.com/eyalamirmusic/NanoTest), pulled in via `FetchContent`. Each `nano::test(...)` is registered as its own CTest entry, so a single test can be run with `./build/Tests/ScopedAllocationsTests --test "name"` or `ctest --test-dir build -R "name"`.

The tests cover the flag/RAII/thread-local mechanics, that the interposed allocators forward correctly when allocation is allowed, and the violation-handler hook (banned `malloc`/`new` route through an installed `std::function`). The default-handler assert path is left to `Examples/Main.cpp`, since asserting would abort the test binary.

`Examples/` and `Tests/` are only added when this is the top-level CMake project (`PROJECT_IS_TOP_LEVEL`). Consumers that pull this in via `add_subdirectory` / `FetchContent` get only the `AllocationsChecker` library and won't trigger the NanoTest fetch.

Code style is enforced by `_clang-format` and `.clang-tidy` at the repo root — apply `clang-format` to any new/edited source.

## Architecture

The library is a thread-scoped allocation guard for real-time threads (the canonical use case is an audio callback that must not allocate). It is intentionally tiny: three `.cpp` files plus one header in `Source/ScopedMemoryAllocations/`, exposed as the CMake INTERFACE target `AllocationsChecker`.

The mechanism has three layers, and changes usually need to keep all three consistent:

1. **Per-thread allow flag (`Allocations.{h,cpp}`).** A `thread_local bool` accessed via `getAllocStatus()`. `EA::Allocations::ScopedSetter` is RAII over this flag — constructing it on a thread bans allocations until it goes out of scope. Because the flag is `thread_local`, banning on the audio thread does not affect the message thread (see `Examples/Main.cpp`).

2. **libc interposition (`Malloc.cpp`).** Defines `extern "C"` `malloc`/`calloc`/`realloc`/`free` that resolve the real libc symbols lazily via `dlsym(RTLD_NEXT, ...)` and call `EA::Allocations::onAllocationViolation()` if the flag is currently `false` before forwarding. This is why the library links against `dl` and currently only works where `RTLD_NEXT` is available (Linux/macOS — not Windows). The lazy `initFunc` pattern is required because `dlsym` itself can be called early in process startup before any state is set up.

3. **Global `new`/`delete` (`Operators.cpp`).** Routes `operator new`/`operator new[]`/`operator delete`/`operator delete[]` through the interposed `malloc`/`free`, so the same assert fires for C++ allocations.

**Violation hook.** `setViolationHandler(std::function<void()>)` overrides what happens on a banned allocation; passing an empty handler restores the default (an `assert`). Tests use this to record violations instead of aborting. `onAllocationViolation()` saves the current allow-flag, flips it to `true` for the duration of the handler call (so the handler can log/record without re-tripping itself), then restores it — so the call is transparent whether triggered from a banned allocation site or invoked directly.

Two non-obvious constraints:

- The library used to avoid all STL headers in its implementation (see commit `831f0c5`); `<functional>` was added for the violation hook, but the rest of the implementation should still stay lean (`<cstdlib>`, `<dlfcn.h>`, `<cassert>`, `<utility>`, `<new>`). Do not introduce STL containers/streams — they could allocate during the very init paths this library hooks.
- Out of the box, the only failure mode is the default handler's `assert`, which compiles out under `NDEBUG`. Anyone needing release-time enforcement should install a handler that traps unconditionally.
