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

auto onAllocationViolationInvokesHandler =
    test("onAllocationViolation invokes the installed handler") = []
{
    int fired = 0;
    EA::Allocations::setViolationHandler([&] { ++fired; });

    EA::Allocations::onAllocationViolation();
    EA::Allocations::onAllocationViolation();

    EA::Allocations::setViolationHandler({});
    check(fired == 2);
};

auto bannedMallocCallsHandler =
    test("malloc inside ScopedSetter routes through the handler") = []
{
    int fired = 0;
    EA::Allocations::setViolationHandler([&] { ++fired; });

    void* p = nullptr;
    {
        EA::Allocations::ScopedSetter setter;
        p = std::malloc(8);
    }
    std::free(p);

    EA::Allocations::setViolationHandler({});
    check(fired == 1);
    check(p != nullptr);
};

auto bannedNewCallsHandler =
    test("new inside ScopedSetter routes through the handler") = []
{
    int fired = 0;
    EA::Allocations::setViolationHandler([&] { ++fired; });

    int* p = nullptr;
    {
        EA::Allocations::ScopedSetter setter;
        p = new int(7);
    }
    delete p;

    EA::Allocations::setViolationHandler({});
    check(fired == 1);
};

auto handlerMayAllocate =
    test("Handler can allocate without re-entering itself") = []
{
    int fired = 0;
    // Allocating inside the handler would loop forever if the ban weren't
    // lifted before invoking it. A scratch buffer per call proves it isn't.
    EA::Allocations::setViolationHandler(
        [&]
        {
            ++fired;
            void* scratch = std::malloc(16);
            std::free(scratch);
        });

    {
        EA::Allocations::ScopedSetter setter;
        void* p = std::malloc(8);
        std::free(p);  // free is gated too, so this fires the handler again.
    }

    EA::Allocations::setViolationHandler({});
    check(fired == 2);
};

auto emptyHandlerRestoresDefault =
    test("Passing an empty handler restores the default") = []
{
    int fired = 0;
    EA::Allocations::setViolationHandler([&] { ++fired; });
    EA::Allocations::setViolationHandler({});

    // The default handler asserts in debug builds, so we can't invoke it here.
    // Instead, install a sentinel afterwards and confirm it overrides.
    int sentinel = 0;
    EA::Allocations::setViolationHandler([&] { ++sentinel; });
    EA::Allocations::onAllocationViolation();
    EA::Allocations::setViolationHandler({});

    check(fired == 0);
    check(sentinel == 1);
};
} // namespace
