#include <ScopedMemoryAllocations/Allocations.h>
#include <thread>

void allocatingFunc()
{
    std::vector<int> v;
    v.reserve(5);
}

int main()
{
    auto messageThread = []
    {
        //Won't assert on allocations:
        allocatingFunc();
    };

    auto audioThread = []
    {
        //Will assert on any allocations, including malloc and new():
        EA::Allocations::ScopedSetter setter;
        allocatingFunc();
    };

    auto first = std::thread(messageThread);
    auto second = std::thread(audioThread);

    first.join();
    second.join();
}
