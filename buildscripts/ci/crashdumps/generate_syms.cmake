set(HERE ${CMAKE_CURRENT_LIST_DIR})

# Options
option(SHOW_HELP "Show help message" OFF)
set(DUMPSYMS_BIN "" CACHE STRING "Path to dump_syms binary")
set(BUILD_DIR "" CACHE STRING "Path to build directory")
set(SYMBOLS_DIR "" CACHE STRING "Path to output symbols directory")
set(APP_BIN "" CACHE STRING "Path to app binary")

set(GENERATE_ARCHS "" CACHE STRING "Generate symbols for architectures")
separate_arguments(GENERATE_ARCHS_LIST UNIX_COMMAND "${GENERATE_ARCHS}")

if(SHOW_HELP)
    message(STATUS "Usage: cmake -DSHOW_HELP=ON [OPTIONS] ...")
    message(STATUS "Generate dump symbols")
    message(STATUS " ")
    message(STATUS "    -DDUMPSYMS_BIN=path    path to dump_syms binary, default use path from environment path")
    message(STATUS "    -DBUILD_DIR=path      path to build dir")
    message(STATUS "    -DSYMBOLS_DIR=path    path to output symbols dir, default '../../build.symbols'")
    message(STATUS "    -DAPP_BIN=path     path to mscore binary")
    message(STATUS "    -DGENERATE_ARCHS=archs  generate symbols for architectures")
    message(STATUS "    -DSHOW_HELP=ON        display this help and exit")
    message(STATUS " ")
    message(STATUS "Example:")
    message(STATUS "  cmake -DBUILD_DIR=../../build.debug -DAPP_BIN=../../build.debug/install/bin/app")
    return()
endif()

# Check required variables
if(NOT DUMPSYMS_BIN)
    message(FATAL_ERROR "error: not set DUMPSYMS_BIN")
endif()
if(NOT BUILD_DIR)
    message(FATAL_ERROR "error: not set BUILD_DIR")
endif()
if(NOT SYMBOLS_DIR)
    message(FATAL_ERROR "error: not set SYMBOLS_DIR")
endif()
if(NOT APP_BIN)
    message(FATAL_ERROR "error: not set APP_BIN")
endif()

set(GEN_SCRIPT "${HERE}/posix/generate_breakpad_symbols.py")
if(WIN32)
    set(GEN_SCRIPT "${HERE}/win/generate_breakpad_symbols.py")
endif()

message(STATUS "DUMPSYMS_BIN: ${DUMPSYMS_BIN}")
message(STATUS "BUILD_DIR: ${BUILD_DIR}")
message(STATUS "SYMBOLS_DIR: ${SYMBOLS_DIR}")
message(STATUS "APP_BIN: ${APP_BIN}")

if(NOT GENERATE_ARCHS_LIST)
    message(STATUS "Generate symbols")

    set(GEN_ARGS
        --dumpsyms-bin=${DUMPSYMS_BIN}
        --symbols-dir=${SYMBOLS_DIR}
        --build-dir=${BUILD_DIR}
        --binary=${APP_BIN}
        --clear
        --verbose
    )

    execute_process(
        COMMAND python3 ${GEN_SCRIPT} ${GEN_ARGS} RESULT_VARIABLE result
    )

    if(result)
        message(FATAL_ERROR "Failed to generate dump symbols")
    else()
        message(STATUS "Dump symbols generated successfully")
    endif()
else()
    foreach(ARCH IN LISTS GENERATE_ARCHS_LIST)
        message(STATUS "Generate symbols for ${ARCH}")

        set(GEN_ARGS
            --dumpsyms-bin=${DUMPSYMS_BIN}
            --symbols-dir=${SYMBOLS_DIR}
            --build-dir=${BUILD_DIR}
            --binary=${APP_BIN}
            --arch=${ARCH}
        )

        execute_process(
            COMMAND python3 ${GEN_SCRIPT} ${GEN_ARGS} RESULT_VARIABLE result
        )

        if(result)
            message(FATAL_ERROR "Failed to generate dump symbols")
        else()
            message(STATUS "Dump symbols generated successfully for ${ARCH}")
        endif()
    endforeach()
endif()
