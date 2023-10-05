cmake_minimum_required(VERSION 3.26)
project(ScopedMemoryAllocations)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(Source)

option(Scoped_Allocations_Examples "" ON)

if (Scoped_Allocations_Examples)
    add_subdirectory(Examples)
endif ()