#pragma once

namespace EA::Allocations
{
bool isAllowedToAllocate();
void setAllowedToAllocate(bool canAllocate);

struct ScopedSetter
{
    ScopedSetter();
    ~ScopedSetter();
};

} // namespace EA::Allocations
