add_library(AllocationsChecker INTERFACE)
target_sources(AllocationsChecker INTERFACE
        ScopedMemoryAllocations/Malloc.cpp
        ScopedMemoryAllocations/Allocations.cpp
        ScopedMemoryAllocations/Operators.cpp)

if (SCOPED_ALLOCATIONS_TEST)
    target_compile_definitions(AllocationsChecker INTERFACE EA_SCOPED_ALLOCATIONS_TEST)
    target_link_libraries(AllocationsChecker INTERFACE dl)
endif ()

target_include_directories(AllocationsChecker INTERFACE ${CMAKE_CURRENT_LIST_DIR})
