include(FetchContent)
FetchContent_Declare(
        NanoTest
        GIT_REPOSITORY https://github.com/eyalamirmusic/NanoTest.git
        GIT_TAG main
)
FetchContent_MakeAvailable(NanoTest)

set(platformTests)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(platformTests Tests-Linux.cpp)
endif ()

nano_add_executable(ScopedAllocationsTests
        SOURCES Tests.cpp ${platformTests}
        TARGETS AllocationsChecker)
