#pragma once

#include <functional>

namespace EA::Allocations
{
bool isAllowedToAllocate();
void setAllowedToAllocate(bool canAllocate);

using ViolationHandler = std::function<void()>;

// Install a handler invoked when an allocation happens while a ScopedSetter
// is active. Pass an empty handler to restore the default (which asserts).
void setViolationHandler(ViolationHandler handler);

// Invoked by the interposed allocators when allocation is currently banned.
// Allocations are temporarily re-enabled for the duration of the call so the
// handler can record/log without re-entering itself.
void onAllocationViolation();

struct ScopedSetter
{
    ScopedSetter();
    ~ScopedSetter();
};

} // namespace EA::Allocations
