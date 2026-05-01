include(FetchContent)
FetchContent_Declare(
        NanoTest
        GIT_REPOSITORY https://github.com/eyalamirmusic/NanoTest.git
        GIT_TAG main
)
FetchContent_MakeAvailable(NanoTest)

nano_add_executable(ScopedAllocationsTests
        SOURCES Tests.cpp
        TARGETS AllocationsChecker)
