if(WIN32)
    set(EXEEXT .exe)
endif()

if(NOT UNIX)
    add_test(NAME libFLAC
      COMMAND $<TARGET_FILE:test_libFLAC>
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
    add_test(NAME libFLAC++
      COMMAND $<TARGET_FILE:test_libFLAC++>
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
    add_test(NAME flac_help
      COMMAND $<TARGET_FILE:flacapp> --help
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
    add_test(NAME metaflac_help
      COMMAND $<TARGET_FILE:metaflac> --help
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
    return()
endif()

set(top_srcdir "${PROJECT_SOURCE_DIR}")
set(top_builddir "${PROJECT_BINARY_DIR}")
configure_file(common.sh.in common.sh @ONLY)

set(ALL_TESTS libFLAC grabbag flac metaflac replaygain seeking streams compression)

add_test(NAME libFLAC
    COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/test_libFLAC.sh"
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
if(BUILD_CXXLIBS)
    add_test(NAME libFLAC++
        COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/test_libFLAC++.sh"
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
    list(APPEND ALL_TESTS libFLAC++)
endif()
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/cuesheets")
add_test(NAME grabbag
    COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/test_grabbag.sh"
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
add_test(NAME flac
    COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/test_flac.sh"
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/metaflac-test-files")
add_test(NAME metaflac
    COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/test_metaflac.sh"
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
add_test(NAME replaygain
    COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/test_replaygain.sh"
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
add_test(NAME seeking
    COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/test_seeking.sh"
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
add_test(NAME streams
    COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/test_streams.sh"
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
#   increase this if standard 1500 seconds are not enough
#   set_tests_properties(streams PROPERTIES TIMEOUT 1500)
add_test(NAME compression
    COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/test_compression.sh"
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

set_property(TEST ${ALL_TESTS} APPEND PROPERTY ENVIRONMENT ECHO_C=\\c)
