message(STATUS "Generate dump symbols")

set(HERE ${CMAKE_CURRENT_LIST_DIR})

set(GEN_SCRIPT "${HERE}/generate_syms.cmake")
set(ARTIFACTS_DIR "${CMAKE_SOURCE_DIR}/build.artifacts")
set(SYMBOLS_DIR ${ARTIFACTS_DIR}/symbols)

# Options
set(APP_BIN "" CACHE STRING "Path to app binary")
set(ARCH "" CACHE STRING "System architecture")
set(GENERATE_ARCHS "" CACHE STRING "Generate symbols for architectures")
set(BUILD_DIR "${CMAKE_SOURCE_DIR}/build.release" CACHE STRING "Path to build directory")

if(WIN32)
    file(ARCHIVE_EXTRACT INPUT "${HERE}/win/dump_syms.7z" DESTINATION "${HERE}/win/")
    set(DUMPSYMS_BIN "${HERE}/win/dump_syms.exe")
elseif(LINUX)
    file(ARCHIVE_EXTRACT INPUT "${HERE}/linux/${ARCH}/dump_syms.7z" DESTINATION "${HERE}/linux/")
    set(DUMPSYMS_BIN "${HERE}/linux/dump_syms")
elseif(APPLE)
    file(ARCHIVE_EXTRACT INPUT "${HERE}/macos/dump_syms.7z" DESTINATION "${HERE}/macos/")
    set(DUMPSYMS_BIN "${HERE}/macos/dump_syms")
endif()

set(CONFIG
    -DDUMPSYMS_BIN=${DUMPSYMS_BIN}
    -DBUILD_DIR=${BUILD_DIR}
    -DSYMBOLS_DIR=${SYMBOLS_DIR}
    -DAPP_BIN=${APP_BIN}
    -DGENERATE_ARCHS=${GENERATE_ARCHS}
)

execute_process(
    COMMAND cmake ${CONFIG} -P ${GEN_SCRIPT}
)

execute_process(
    COMMAND ls ${SYMBOLS_DIR} OUTPUT_VARIABLE symbols_dir_contents
)

message(STATUS "SYMBOLS_DIR contents: ${symbols_dir_contents}")
