
project(tests_scene_notation LANGUAGES C CXX)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(${CMAKE_CURRENT_LIST_DIR}/../../../../framework/utests_base/utests_base.cmake)


set(TESTS_SRC
    ${TESTS_BASE_SRC}
    ${CMAKE_CURRENT_LIST_DIR}/notationview_tests.cpp
)


find_package(Qt5 COMPONENTS Core Gui Quick REQUIRED)

add_definitions(

)

include_directories(
    ${CMAKE_CURRENT_LIST_DIR}
    ${TESTS_BASE_INC}
)

set(TESTS_LIBS
    ${TESTS_BASE_LIBS}
)

add_executable(tests_scene_notation_1 ${TESTS_SRC})

target_link_libraries(tests_scene_notation_1 ${TESTS_LIBS} Qt5::Core Qt5::Gui Qt5::Quick)
