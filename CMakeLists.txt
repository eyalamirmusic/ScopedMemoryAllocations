cmake_minimum_required(VERSION 3.29)
project(ScopedMemoryAllocations)

set(CMAKE_CXX_STANDARD 20)

option(SCOPED_ALLOCATIONS_TEST
        "Enable allocation interposition; when OFF the API still exists but malloc/new are not hooked."
        ON)

# Interposition relies on dlsym(RTLD_NEXT, ...), which Windows lacks. Force the
# flag off so the library degrades to its API-only stub and the Examples/Tests
# gate below skips targets that use POSIX-only calls.
if (WIN32)
    set(SCOPED_ALLOCATIONS_TEST OFF)
endif ()

add_subdirectory(Source)

if (PROJECT_IS_TOP_LEVEL AND SCOPED_ALLOCATIONS_TEST)
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
    add_subdirectory(Examples)

    enable_testing()
    add_subdirectory(Tests)
endif ()