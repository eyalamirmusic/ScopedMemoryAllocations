add_library(AllocationsChecker INTERFACE)
target_sources(AllocationsChecker INTERFACE
        ScopedMemoryAllocations/Malloc.cpp
        ScopedMemoryAllocations/Allocations.cpp
        ScopedMemoryAllocations/Operators.cpp)

target_link_libraries(AllocationsChecker INTERFACE dl)
target_include_directories(AllocationsChecker INTERFACE ${CMAKE_CURRENT_LIST_DIR})