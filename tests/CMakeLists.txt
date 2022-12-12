
include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.2.1 # or a later release
)

FetchContent_MakeAvailable(Catch2)

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)

include(CTest)
include(Catch)

add_executable(tests
    test.cpp
    ../src/IEventProcessor.cpp
)
target_link_libraries(tests PRIVATE Catch2::Catch2WithMain)
target_include_directories(tests
    PRIVATE
        ${CMAKE_SOURCE_DIR}
)

catch_discover_tests(tests)

add_executable(benchmark
    benchmark.cpp
    ../src/IEventProcessor.cpp
    ../src/simulation/SingleExecution.cpp
    ../src/simulation/TestEventProducer.cpp
)

target_link_libraries(benchmark
    PRIVATE
    Catch2::Catch2WithMain
)

target_include_directories(benchmark
    PRIVATE
        ${CMAKE_SOURCE_DIR}
)
catch_discover_tests(benchmark)
