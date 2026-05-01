cmake_minimum_required(VERSION 3.29)
project(ScopedMemoryAllocations)

set(CMAKE_CXX_STANDARD 20)

option(Scoped_Allocations_Disable
        "Disable allocation interposition; the API still exists but malloc/new are not hooked."
        OFF)

if (WIN32)
    set(Scoped_Allocations_Disable ON)
endif ()

add_subdirectory(Source)

if (PROJECT_IS_TOP_LEVEL AND NOT Scoped_Allocations_Disable)
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
    add_subdirectory(Examples)

    enable_testing()
    add_subdirectory(Tests)
endif ()