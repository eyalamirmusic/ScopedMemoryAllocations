#include "Allocations.h"

#include <cassert>
#include <utility>

namespace EA::Allocations
{

bool& getAllocStatus()
{
    thread_local auto canAlloc = true;
    return canAlloc;
}

ViolationHandler defaultHandler()
{
    return []
    {
        assert(
            false
            && "Disallowed allocation while EA::Allocations::ScopedSetter was active");
    };
}

ViolationHandler& getHandler()
{
    static auto handler = defaultHandler();
    return handler;
}

bool isAllowedToAllocate()
{ return getAllocStatus(); }

void setAllowedToAllocate(bool canAllocate)
{ getAllocStatus() = canAllocate; }

void setViolationHandler(ViolationHandler handler)
{ getHandler() = handler ? std::move(handler) : defaultHandler(); }

void onAllocationViolation()
{
    auto& flag = getAllocStatus();
    const auto previous = flag;
    flag = true;
    getHandler()();
    flag = previous;
}

ScopedSetter::ScopedSetter()
    : previous(getAllocStatus())
{ setAllowedToAllocate(false); }

ScopedSetter::~ScopedSetter()
{ setAllowedToAllocate(previous); }
} // namespace EA::Allocations
