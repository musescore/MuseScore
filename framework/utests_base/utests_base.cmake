
define_property(TARGET PROPERTY OUTPUT_XML
    BRIEF_DOCS "List XML files outputed by google test."
    FULL_DOCS "List XML files outputed by google test."
)

cmake_policy(SET CMP0079 NEW)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../../thirdparty/googletest googletest)
enable_testing()

get_property(gmock_LIBS GLOBAL PROPERTY gmock_LIBS)
get_property(gmock_INCLUDE_DIRS GLOBAL PROPERTY gmock_INCLUDE_DIRS)

include_directories(
    ${gmock_INCLUDE_DIRS}
    ${CMAKE_CURRENT_LIST_DIR}
)

set(TESTS_BASE_SRC
    ${CMAKE_CURRENT_LIST_DIR}/main.cpp
)

set(TESTS_BASE_INC
    ${gmock_INCLUDE_DIRS}
    ${CMAKE_CURRENT_LIST_DIR}
)

set(TESTS_BASE_LIBS
   gmock
)


