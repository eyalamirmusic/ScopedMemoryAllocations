#include <NanoTest/NanoTest.h>
#include <ScopedMemoryAllocations/Allocations.h>

#include <cstdlib>
#include <dlfcn.h>

extern "C" void (*originalFree)(void*);

namespace
{
using namespace nano;

// Leaves a pending dlerror string behind, allocated through the interposed
// malloc. The next dlsym frees it (glibc's _dlerror_run).
void failLookup()
{ (void) dlsym(RTLD_DEFAULT, "ea_scoped_allocations_no_such_symbol"); }

auto freeResolvesAfterFailedDlopen =
    test("free resolves while a dlerror is pending") = []
{
    // glibc 2.44's dlsym frees the pending error string, re-entering free.
    (void) dlopen("libdoes-not-exist.so.0", RTLD_NOW);

    originalFree = nullptr;
    std::free(std::malloc(8));

    check(originalFree != nullptr);
};

auto firstFreeIsAPendingDlerror =
    test("A first free of a pending dlerror string frees it once") = []
{
    failLookup();

    // As if this were the process's first free, as when a library's static
    // constructor (libudev's) fails a lookup and then looks up again:
    // _dlerror_run frees the pending string while it is still pending.
    // Resolving the real free must not re-enter dlsym, which would free the
    // same string a second time.
    originalFree = nullptr;
    failLookup();

    check(originalFree != nullptr);
};

auto bannedMallocCaughtAfterDlerror =
    test("malloc is still intercepted after a pending dlerror") = []
{
    failLookup();
    failLookup();

    auto fired = 0;
    EA::Allocations::setViolationHandler([&] { ++fired; });

    auto* p = static_cast<void*>(nullptr);
    {
        auto setter = EA::Allocations::ScopedSetter();
        p = std::malloc(8);
    }
    std::free(p);

    EA::Allocations::setViolationHandler({});
    check(fired == 1);
    check(p != nullptr);
};
} // namespace
