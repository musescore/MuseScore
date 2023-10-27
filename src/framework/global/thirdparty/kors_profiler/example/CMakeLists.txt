cmake_minimum_required(VERSION 3.14)

project(profiler_example LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(${CMAKE_CURRENT_LIST_DIR}/../profiler.cmake)

include_directories(${KORS_PROFILER_INC})
add_definitions(-DKORS_PROFILER_ENABLED)

add_executable(${PROJECT_NAME}
    main.cpp
    ${KORS_PROFILER_SRC}
)

target_link_libraries(${PROJECT_NAME}
    pthread
)
