if(WIN32)
    set(EXEEXT .exe)
endif()

if(NOT UNIX)
    find_program(BUSYBOX_EXE busybox)
    if(NOT BUSYBOX_EXE)
        message(STATUS "Could NOT find busybox, only basic tests are enabled")
        add_test(NAME libFLAC
          COMMAND $<TARGET_FILE:test_libFLAC>
          WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
        if(BUILD_CXXLIBS)
            add_test(NAME libFLAC++
              COMMAND $<TARGET_FILE:test_libFLAC++>
              WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
        endif()
        if(BUILD_PROGRAMS)
            add_test(NAME flac_help
              COMMAND $<TARGET_FILE:flacapp> --help
              WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
            add_test(NAME flac_utf8_test COMMAND $<TARGET_FILE:flacapp> -t
              "${CMAKE_CURRENT_SOURCE_DIR}/flac-to-flac-metadata-test-files/שלום.flac"
              "${CMAKE_CURRENT_SOURCE_DIR}/flac-to-flac-metadata-test-files/Prøve.flac"
              "${CMAKE_CURRENT_SOURCE_DIR}/flac-to-flac-metadata-test-files/🤔.flac")
            add_test(NAME metaflac_help
              COMMAND $<TARGET_FILE:metaflac> --help
              WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
        endif()
        return()
    else()
        set(BUSYBOX_CMD "sh")
    endif()
endif()

set(top_srcdir "${PROJECT_SOURCE_DIR}")
set(top_builddir "${PROJECT_BINARY_DIR}")
if(NOT GIT_COMMIT_TAG AND GIT_COMMIT_HASH AND GIT_COMMIT_DATE)
    set(GIT_COMMIT_VERSION_HASH yes)
endif()

configure_file(common.sh.in common.sh @ONLY)

set(ALL_TESTS libFLAC grabbag)

add_test(NAME libFLAC
    COMMAND ${BUSYBOX_EXE} ${BUSYBOX_CMD} "${CMAKE_CURRENT_SOURCE_DIR}/test_libFLAC.sh"
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
if(BUILD_CXXLIBS)
    add_test(NAME libFLAC++
        COMMAND ${BUSYBOX_EXE} ${BUSYBOX_CMD} "${CMAKE_CURRENT_SOURCE_DIR}/test_libFLAC++.sh"
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
    list(APPEND ALL_TESTS libFLAC++)
    set_tests_properties(libFLAC++ PROPERTIES DEPENDS libFLAC)
endif()
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/cuesheets")
add_test(NAME grabbag
    COMMAND ${BUSYBOX_EXE} ${BUSYBOX_CMD} "${CMAKE_CURRENT_SOURCE_DIR}/test_grabbag.sh"
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
if(BUILD_PROGRAMS)
    add_test(NAME generate_streams
        COMMAND ${BUSYBOX_EXE} ${BUSYBOX_CMD} "${CMAKE_CURRENT_SOURCE_DIR}/generate_streams.sh"
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
    add_test(NAME flac
        COMMAND ${BUSYBOX_EXE} ${BUSYBOX_CMD} "${CMAKE_CURRENT_SOURCE_DIR}/test_flac.sh"
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
    # UTF-8 must be tested separately on Windows because busybox-w32 doesn't support UTF-8
    if(BUSYBOX_EXE)
        add_test(NAME flac_utf8_test COMMAND $<TARGET_FILE:flacapp> -t
            "${CMAKE_CURRENT_SOURCE_DIR}/flac-to-flac-metadata-test-files/שלום.flac"
            "${CMAKE_CURRENT_SOURCE_DIR}/flac-to-flac-metadata-test-files/Prøve.flac"
            "${CMAKE_CURRENT_SOURCE_DIR}/flac-to-flac-metadata-test-files/🤔.flac")
        list(APPEND ALL_TESTS flac_utf8_test)
    endif()
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/metaflac-test-files")
    add_test(NAME metaflac
        COMMAND ${BUSYBOX_EXE} ${BUSYBOX_CMD} "${CMAKE_CURRENT_SOURCE_DIR}/test_metaflac.sh"
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
    add_test(NAME replaygain
        COMMAND ${BUSYBOX_EXE} ${BUSYBOX_CMD} "${CMAKE_CURRENT_SOURCE_DIR}/test_replaygain.sh"
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
    add_test(NAME seeking
        COMMAND ${BUSYBOX_EXE} ${BUSYBOX_CMD} "${CMAKE_CURRENT_SOURCE_DIR}/test_seeking.sh"
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
    add_test(NAME streams
        COMMAND ${BUSYBOX_EXE} ${BUSYBOX_CMD} "${CMAKE_CURRENT_SOURCE_DIR}/test_streams.sh"
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
    #   increase this if standard 1500 seconds are not enough
    #   set_tests_properties(streams PROPERTIES TIMEOUT 1500)
    add_test(NAME compression
        COMMAND ${BUSYBOX_EXE} ${BUSYBOX_CMD} "${CMAKE_CURRENT_SOURCE_DIR}/test_compression.sh"
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
    set_tests_properties(flac metaflac replaygain seeking streams compression PROPERTIES DEPENDS generate_streams)
    list(APPEND ALL_TESTS generate_streams flac metaflac replaygain seeking streams compression)
endif()


set_property(TEST ${ALL_TESTS} APPEND PROPERTY ENVIRONMENT FLAC__TEST_LEVEL=1)
if(BUSYBOX_EXE)
    set_property(TEST ${ALL_TESTS} APPEND PROPERTY ENVIRONMENT ECHO_N=-n)
    set_property(TEST ${ALL_TESTS} APPEND PROPERTY ENVIRONMENT WIN32BUSYBOX=yes)
else()
    set_property(TEST ${ALL_TESTS} APPEND PROPERTY ENVIRONMENT ECHO_C=\\c)
endif()
