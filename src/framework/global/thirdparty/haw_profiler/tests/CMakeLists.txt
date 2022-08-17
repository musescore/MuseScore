cmake_minimum_required(VERSION 3.14)

project(haw_profiler LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(${CMAKE_CURRENT_LIST_DIR}/../src/profiler.cmake)

include_directories(${HAW_PROFILER_INC})

add_executable(haw_profiler
    main.cpp
    ${HAW_PROFILER_SRC}
)

target_link_libraries(haw_profiler
    pthread
)
