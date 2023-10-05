#include "Allocations.h"

#include <atomic>
#include <thread>
#include <array>

namespace EA::Allocations
{
struct Status
{
    std::thread::id id {};
    bool canAllocate = true;
};

struct ThreadMap
{
    Status& get(std::thread::id id)
    {
        std::lock_guard guard(mutex);

        for (size_t index = 0; index < actualSize; ++index)
        {
            auto& current = map[index];

            if (current.id == id)
                return current;
        }

        if (actualSize + 1 < map.size())
        {
            auto& current = map[actualSize];
            current.id = id;
            ++actualSize;

            return current;
        }

        return map[0];
    }

    std::mutex mutex;

    size_t actualSize = 0;
    std::array<Status, 100000> map;
};

bool& getCurentStatus()
{
    static ThreadMap statusMap;
    auto id = std::this_thread::get_id();

    auto& status = statusMap.get(id);
    return status.canAllocate;
}

bool isAllowedToAllocate()
{
    return getCurentStatus();
}

void setAllowedToAllocate(bool canAllocate)
{
    getCurentStatus() = canAllocate;
}

Allocations::ScopedSetter::ScopedSetter()
{
    setAllowedToAllocate(false);
}

Allocations::ScopedSetter::~ScopedSetter()
{
    setAllowedToAllocate(true);
}
} // namespace EA::Allocations
