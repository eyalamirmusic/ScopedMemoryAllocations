#include <NanoTest/NanoTest.h>

#include <cstdlib>
#include <dlfcn.h>

extern "C" void (*originalFree)(void*);

namespace
{
using namespace nano;

auto freeResolvesAfterFailedDlopen =
    test("free resolves while a dlerror is pending") = []
{
    // glibc 2.44's dlsym frees the pending error string, re-entering free.
    (void) dlopen("libdoes-not-exist.so.0", RTLD_NOW);

    originalFree = nullptr;
    std::free(std::malloc(8));

    check(originalFree != nullptr);
};
} // namespace
