#include <ScopedMemoryAllocations/Allocations.h>
#include <thread>

void* mem;

void allocatingFunc()
{
    realloc(mem, 25);
}

int main()
{
    mem = malloc(25);

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
