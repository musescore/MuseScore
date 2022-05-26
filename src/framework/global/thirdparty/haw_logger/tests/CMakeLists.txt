cmake_minimum_required(VERSION 3.14)

project(haw_logger LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5Core)

add_definitions(-DHAW_LOGGER_QT_SUPPORT)
include(${CMAKE_CURRENT_LIST_DIR}/../log/logger.cmake)

include_directories(${HAW_LOGGER_INC})

add_executable(haw_logger
    main.cpp
    ${HAW_LOGGER_SRC}
)

target_link_libraries(haw_logger
    stdc++fs
    pthread
    Qt${QT_VERSION_MAJOR}::Core
)
