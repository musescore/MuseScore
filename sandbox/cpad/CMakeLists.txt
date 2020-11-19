cmake_minimum_required(VERSION 3.5)

project(cpad LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "-g")

set(MU_ROOT ../..)
set(CRASHPAD_PATH ${MU_ROOT}/thirdparty/google_crashpad_client)

add_subdirectory(${CRASHPAD_PATH} google_crashpad_client)

add_executable(cpad
    main.cpp
)

target_include_directories(cpad PUBLIC
    ${CRASHPAD_PATH}
    ${CRASHPAD_PATH}/third_party/mini_chromium/mini_chromium
)

target_link_libraries(cpad
    gcrashpad
)
