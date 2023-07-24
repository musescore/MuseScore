macro ( ADD_FLUID_TEST _test )
    add_executable( ${_test} ${_test}.c )

    # only build this unit test when explicitly requested by "make check"
    set_target_properties(${_test} PROPERTIES EXCLUDE_FROM_ALL TRUE)

    # import necessary compile flags and dependency libraries
    if ( FLUID_CPPFLAGS )
        set_target_properties ( ${_test} PROPERTIES COMPILE_FLAGS ${FLUID_CPPFLAGS} )
    endif ( FLUID_CPPFLAGS )
    target_link_libraries( ${_test} libfluidsynth-OBJ )

    # use the local include path to look for fluidsynth.h, as we cannot be sure fluidsynth is already installed
    target_include_directories(${_test}
    PUBLIC
    $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include> # include auto generated headers
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include> # include "normal" public (sub-)headers
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src> # include private headers
    $<TARGET_PROPERTY:libfluidsynth-OBJ,INCLUDE_DIRECTORIES> # include all other header search paths needed by libfluidsynth (esp. glib)
    )

    # add the test to ctest
    ADD_TEST(NAME ${_test} COMMAND ${_test})

    # append the current unit test to check-target as dependency
    add_dependencies(check ${_test})

endmacro ( ADD_FLUID_TEST )

macro ( ADD_FLUID_TEST_UTIL _util )
    add_executable( ${_util} ${_util}.c )

    # only build this unit test when explicitly requested by "make check"
    set_target_properties(${_util} PROPERTIES EXCLUDE_FROM_ALL TRUE)

    # append no-op generator expression to avoid VS or XCode from adding per-config subdirectories
    set_target_properties(${_util} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/test/utils/$<0:>)

    # import necessary compile flags and dependency libraries
    if ( FLUID_CPPFLAGS )
        set_target_properties ( ${_util} PROPERTIES COMPILE_FLAGS ${FLUID_CPPFLAGS} )
    endif ( FLUID_CPPFLAGS )
    target_link_libraries( ${_util} libfluidsynth-OBJ )

    # use the local include path to look for fluidsynth.h, as we cannot be sure fluidsynth is already installed
    target_include_directories(${_util}
    PUBLIC
    $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include> # include auto generated headers
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include> # include "normal" public (sub-)headers
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src> # include private headers
    $<TARGET_PROPERTY:libfluidsynth-OBJ,INCLUDE_DIRECTORIES> # include all other header search paths needed by libfluidsynth (esp. glib)
    )

    # append the current unit test to check-target as dependency
    add_dependencies(check ${_util})

endmacro ( ADD_FLUID_TEST_UTIL )

# This macro adds a test that writes its output to a file called
# <test>.output (in the current working dir) and then compares
# the content with the file given in _expected_output
macro ( ADD_FLUID_SF_DUMP_TEST _sfname)

    set( test_args "${PROJECT_SOURCE_DIR}/sf2/${_sfname} ${_sfname}.yml" )

    ADD_TEST(${_sfname}_dump_test
        ${CMAKE_COMMAND}
        -Dtest_cmd=${PROJECT_BINARY_DIR}/test/utils/dump_sfont${CMAKE_EXECUTABLE_SUFFIX}
        -Dtest_args=${test_args}
        -Dtest_output=${_sfname}.yml
        -Dexpected_output=${PROJECT_SOURCE_DIR}/sf2/${_sfname}.yml
        -P ${PROJECT_SOURCE_DIR}/cmake_admin/RunOutputTest.cmake
    )

endmacro ( ADD_FLUID_SF_DUMP_TEST )

macro ( ADD_FLUID_DEMO _demo )

    if ( ${ARGC} GREATER 1 )
        string( TOLOWER "${ARGV1}" _LANGEXT )
    else ()
        set( _LANGEXT "c" )
    endif ()

    ADD_EXECUTABLE(${_demo} ${_demo}.${_LANGEXT} )

    # only build this unit test when explicitly requested by "make check"
    set_target_properties(${_demo} PROPERTIES EXCLUDE_FROM_ALL TRUE)

    # request C++11 features only for the C++ example(s)
    if ( "${_LANGEXT}" STREQUAL "cxx" )
        target_compile_features( ${_demo} PUBLIC cxx_std_11 )
        set_target_properties( ${_demo} PROPERTIES CXX_EXTENSIONS OFF )
    endif()

    # import necessary compile flags and dependency libraries
    if ( FLUID_CPPFLAGS )
        set_target_properties ( ${_demo} PROPERTIES COMPILE_FLAGS ${FLUID_CPPFLAGS} )
    endif ( FLUID_CPPFLAGS )
    TARGET_LINK_LIBRARIES(${_demo} libfluidsynth)

    # use the local include path to look for fluidsynth.h, as we cannot be sure fluidsynth is already installed
    target_include_directories(${_demo}
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include> # include auto generated headers
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include> # include "normal" public (sub-)headers
        $<TARGET_PROPERTY:libfluidsynth,INCLUDE_DIRECTORIES> # include all other header search paths needed by libfluidsynth (esp. glib)
    )

    # append the current unit test to check-target as dependency
    add_dependencies(demo ${_demo})

endmacro ( ADD_FLUID_DEMO )
