option(Scoped_Allocations_Disable
        "Disable allocation interposition; the API still exists but malloc/new are not hooked."
        OFF)

add_library(AllocationsChecker INTERFACE)
target_sources(AllocationsChecker INTERFACE
        ScopedMemoryAllocations/Malloc.cpp
        ScopedMemoryAllocations/Allocations.cpp
        ScopedMemoryAllocations/Operators.cpp)

if (WIN32 OR Scoped_Allocations_Disable)
    target_compile_definitions(AllocationsChecker INTERFACE EA_SCOPED_ALLOCATIONS_DISABLE)
endif ()

if (NOT WIN32)
    target_link_libraries(AllocationsChecker INTERFACE dl)
endif ()

target_include_directories(AllocationsChecker INTERFACE ${CMAKE_CURRENT_LIST_DIR})
