#include <NanoTest/NanoTest.h>
#include <ScopedMemoryAllocations/Allocations.h>

#include <atomic>
#include <cstdlib>
#include <thread>

using namespace nano;

namespace
{
auto defaultAllowed = test("Default state permits allocation") = []
{
    check(EA::Allocations::isAllowedToAllocate());
};

auto setterToggles = test("setAllowedToAllocate toggles the flag") = []
{
    EA::Allocations::setAllowedToAllocate(false);
    const bool whileBanned = EA::Allocations::isAllowedToAllocate();
    EA::Allocations::setAllowedToAllocate(true);
    const bool afterRestore = EA::Allocations::isAllowedToAllocate();

    check(!whileBanned);
    check(afterRestore);
};

auto scopedSetterBlocksInScope = test("ScopedSetter bans allocation within its scope") = []
{
    bool inScope = true;
    {
        EA::Allocations::ScopedSetter setter;
        inScope = EA::Allocations::isAllowedToAllocate();
    }
    const bool afterScope = EA::Allocations::isAllowedToAllocate();

    check(!inScope);
    check(afterScope);
};

auto flagIsPerThread = test("Allocation flag is thread-local") = []
{
    std::atomic<bool> workerSawBan{false};

    auto worker = std::thread(
        [&]
        {
            EA::Allocations::ScopedSetter setter;
            workerSawBan = !EA::Allocations::isAllowedToAllocate();
        });
    worker.join();

    // The worker banned allocations on its own thread; the main thread's
    // thread_local flag must be untouched.
    check(workerSawBan);
    check(EA::Allocations::isAllowedToAllocate());
};

auto mallocFreeWork = test("malloc/free pass through when allowed") = []
{
    void* p = std::malloc(64);
    const bool ok = (p != nullptr);
    std::free(p);
    check(ok);
};

auto callocZeroInits = test("calloc returns zero-initialised memory") = []
{
    constexpr std::size_t count = 8;
    auto* p = static_cast<unsigned char*>(std::calloc(count, 1));

    bool allZero = (p != nullptr);
    for (std::size_t i = 0; allZero && i < count; ++i)
        if (p[i] != 0)
            allZero = false;

    std::free(p);
    check(allZero);
};

auto reallocGrows = test("realloc grows a buffer") = []
{
    void* p = std::malloc(8);
    p = std::realloc(p, 64);
    const bool ok = (p != nullptr);
    std::free(p);
    check(ok);
};

auto newDeleteWork = test("new/delete pass through when allowed") = []
{
    int* p = new int(42);
    const bool ok = (p != nullptr && *p == 42);
    delete p;
    check(ok);
};

auto newArrayDeleteWork = test("new[]/delete[] pass through when allowed") = []
{
    int* p = new int[4]{1, 2, 3, 4};
    const bool ok = (p != nullptr && p[3] == 4);
    delete[] p;
    check(ok);
};
} // namespace
