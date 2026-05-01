#include <NanoTest/NanoTest.h>
#include <ScopedMemoryAllocations/Allocations.h>

#include <atomic>
#include <cstdlib>
#include <thread>

using namespace nano;

namespace
{
auto defaultAllowed = test("Default state permits allocation") = []
{ check(EA::Allocations::isAllowedToAllocate()); };

auto setterToggles = test("setAllowedToAllocate toggles the flag") = []
{
    EA::Allocations::setAllowedToAllocate(false);
    const auto whileBanned = EA::Allocations::isAllowedToAllocate();
    EA::Allocations::setAllowedToAllocate(true);
    const auto afterRestore = EA::Allocations::isAllowedToAllocate();

    check(!whileBanned);
    check(afterRestore);
};

auto scopedSetterBlocksInScope =
    test("ScopedSetter bans allocation within its scope") = []
{
    auto inScope = true;
    {
        auto setter = EA::Allocations::ScopedSetter();
        inScope = EA::Allocations::isAllowedToAllocate();
    }
    const auto afterScope = EA::Allocations::isAllowedToAllocate();

    check(!inScope);
    check(afterScope);
};

auto flagIsPerThread = test("Allocation flag is thread-local") = []
{
    auto workerSawBan = std::atomic<bool> {false};

    auto worker = std::thread(
        [&]
        {
            auto setter = EA::Allocations::ScopedSetter();
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
    auto* p = std::malloc(64);
    const auto ok = (p != nullptr);
    std::free(p);
    check(ok);
};

auto callocZeroInits = test("calloc returns zero-initialised memory") = []
{
    constexpr auto count = std::size_t {8};
    auto* p = static_cast<unsigned char*>(std::calloc(count, 1));

    auto allZero = (p != nullptr);
    for (auto i = std::size_t {0}; allZero && i < count; ++i)
        if (p[i] != 0)
            allZero = false;

    std::free(p);
    check(allZero);
};

auto reallocGrows = test("realloc grows a buffer") = []
{
    auto* p = std::malloc(8);
    p = std::realloc(p, 64);
    const auto ok = (p != nullptr);
    std::free(p);
    check(ok);
};

auto newDeleteWork = test("new/delete pass through when allowed") = []
{
    auto* p = new int(42);
    const auto ok = (p != nullptr && *p == 42);
    delete p;
    check(ok);
};

auto newArrayDeleteWork = test("new[]/delete[] pass through when allowed") = []
{
    auto* p = new int[4] {1, 2, 3, 4};
    const auto ok = (p != nullptr && p[3] == 4);
    delete[] p;
    check(ok);
};

auto onAllocationViolationInvokesHandler =
    test("onAllocationViolation invokes the installed handler") = []
{
    auto fired = 0;
    EA::Allocations::setViolationHandler([&] { ++fired; });

    EA::Allocations::onAllocationViolation();
    EA::Allocations::onAllocationViolation();

    EA::Allocations::setViolationHandler({});
    check(fired == 2);
};

auto bannedMallocCallsHandler =
    test("malloc inside ScopedSetter routes through the handler") = []
{
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

auto bannedNewCallsHandler =
    test("new inside ScopedSetter routes through the handler") = []
{
    auto fired = 0;
    EA::Allocations::setViolationHandler([&] { ++fired; });

    auto* p = static_cast<int*>(nullptr);
    {
        auto setter = EA::Allocations::ScopedSetter();
        p = new int(7);
    }
    delete p;

    EA::Allocations::setViolationHandler({});
    check(fired == 1);
};

auto handlerMayAllocate =
    test("Handler can allocate without re-entering itself") = []
{
    auto fired = 0;
    // Allocating inside the handler would loop forever if the ban weren't
    // lifted before invoking it. A scratch buffer per call proves it isn't.
    EA::Allocations::setViolationHandler(
        [&]
        {
            ++fired;
            auto* scratch = std::malloc(16);
            std::free(scratch);
        });

    {
        auto setter = EA::Allocations::ScopedSetter();
        auto* p = std::malloc(8);
        std::free(p); // free is gated too, so this fires the handler again.
    }

    EA::Allocations::setViolationHandler({});
    check(fired == 2);
};

auto emptyHandlerRestoresDefault =
    test("Passing an empty handler restores the default") = []
{
    auto fired = 0;
    EA::Allocations::setViolationHandler([&] { ++fired; });
    EA::Allocations::setViolationHandler({});

    // The default handler asserts in debug builds, so we can't invoke it here.
    // Instead, install a sentinel afterwards and confirm it overrides.
    auto sentinel = 0;
    EA::Allocations::setViolationHandler([&] { ++sentinel; });
    EA::Allocations::onAllocationViolation();
    EA::Allocations::setViolationHandler({});

    check(fired == 0);
    check(sentinel == 1);
};

auto nestedScopedSetterPreservesOuterBan = test(
    "Nested ScopedSetter restores the outer flag, not unconditionally true") = []
{
    {
        auto outer = EA::Allocations::ScopedSetter();
        {
            auto inner = EA::Allocations::ScopedSetter();
            check(!EA::Allocations::isAllowedToAllocate());
        }
        // After ~inner, outer's ban must still hold.
        check(!EA::Allocations::isAllowedToAllocate());
    }
    check(EA::Allocations::isAllowedToAllocate());
};

auto alignedNewIsCaught =
    test("Aligned new inside ScopedSetter routes through the handler") = []
{
    struct alignas(64) Aligned
    {
        char data[128];
    };

    auto fired = 0;
    EA::Allocations::setViolationHandler([&] { ++fired; });

    auto* p = static_cast<Aligned*>(nullptr);
    {
        auto setter = EA::Allocations::ScopedSetter();
        p = new Aligned();
    }
    const auto aligned =
        (reinterpret_cast<std::uintptr_t>(p) % alignof(Aligned)) == 0;
    delete p;

    EA::Allocations::setViolationHandler({});
    check(fired == 1);
    check(aligned);
};

auto posixMemalignIsCaught =
    test("posix_memalign inside ScopedSetter routes through the handler") = []
{
    auto fired = 0;
    EA::Allocations::setViolationHandler([&] { ++fired; });

    auto* p = static_cast<void*>(nullptr);
    {
        auto setter = EA::Allocations::ScopedSetter();
        (void) posix_memalign(&p, 64, 128);
    }
    std::free(p);

    EA::Allocations::setViolationHandler({});
    check(fired == 1);
    check(p != nullptr);
};
} // namespace
