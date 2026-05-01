cmake_minimum_required(VERSION 3.26)
project(ScopedMemoryAllocations)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(Source)

if (PROJECT_IS_TOP_LEVEL AND NOT WIN32 AND NOT Scoped_Allocations_Disable)
    add_subdirectory(Examples)

    enable_testing()
    add_subdirectory(Tests)
endif ()