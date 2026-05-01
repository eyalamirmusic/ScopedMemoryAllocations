#include "Allocations.h"

#include <cassert>
#include <utility>

namespace EA::Allocations
{

bool& getAllocStatus()
{
    thread_local bool canAlloc = true;
    return canAlloc;
}

ViolationHandler defaultHandler()
{
    return []
    {
        assert(false && "Disallowed allocation while EA::Allocations::ScopedSetter was active");
    };
}

ViolationHandler& getHandler()
{
    static ViolationHandler handler = defaultHandler();
    return handler;
}

bool isAllowedToAllocate()
{
    return getAllocStatus();
}

void setAllowedToAllocate(bool canAllocate)
{
    getAllocStatus() = canAllocate;
}

void setViolationHandler(ViolationHandler handler)
{
    getHandler() = handler ? std::move(handler) : defaultHandler();
}

void onAllocationViolation()
{
    auto& flag = getAllocStatus();
    const bool previous = flag;
    flag = true;
    getHandler()();
    flag = previous;
}

ScopedSetter::ScopedSetter()
{
    setAllowedToAllocate(false);
}

ScopedSetter::~ScopedSetter()
{
    setAllowedToAllocate(true);
}
} // namespace EA::Allocations
