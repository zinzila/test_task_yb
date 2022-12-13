cmake_minimum_required(VERSION 3.0.0)
project(youbourse VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)

option(TSAN "Enable ThreadSanitizer" OFF)
option(GPROF "Enable gprof" OFF)

if(TSAN AND GPROF)
    message(FATAL_ERROR "Cannot use TSAN and GPROF simultaneously")
endif()

if(TSAN)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=thread")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=thread")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=thread")

    message(STATUS "ThreadSanitizer enabled")
endif()

if(GPROF)
    set(CMAKE_BUILD_TYPE RelWithDebInfo)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pg")
endif()

add_executable(youbourse
    main.cpp
    src/IEventProcessor.cpp
    src/simulation/TestEventProducer.cpp
    src/simulation/SingleExecution.cpp
)

target_include_directories(youbourse
    PRIVATE
        ${CMAKE_SOURCE_DIR}
)

enable_testing()
add_subdirectory(tests)
