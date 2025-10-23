cmake_minimum_required(VERSION 3.5)

project(rpcqueue_tests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)


include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY git@github.com:google/googletest.git
  GIT_TAG        v1.17.0
)
FetchContent_MakeAvailable(googletest)

include(${CMAKE_CURRENT_LIST_DIR}/../rpcqueue/rpcqueue.cmake)

add_compile_options("-fsanitize=address")
add_compile_options("-fno-omit-frame-pointer")
add_link_options("-fsanitize=address")

add_executable(${PROJECT_NAME}
    ${KORS_RPCQUEUE_SRC}
    ringqueue_tests.cpp
    rpcqueue_tests.cpp
)
target_link_libraries(${PROJECT_NAME} gtest_main)

