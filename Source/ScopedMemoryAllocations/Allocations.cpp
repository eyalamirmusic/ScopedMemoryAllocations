#include "Allocations.h"

#include <atomic>
#include <thread>
#include <array>

namespace EA::Allocations
{

bool& getAllocStatus()
{
    thread_local bool canAlloc = true;
    return canAlloc;
}


bool isAllowedToAllocate()
{
    return getAllocStatus();
}

void setAllowedToAllocate(bool canAllocate)
{
    getAllocStatus() = canAllocate;
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
