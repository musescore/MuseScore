if(NOT DEFINED CMAKE_SCRIPT_MODE_FILE)
    message(FATAL_ERROR "This file is a script")
endif()

include(${CMAKE_CURRENT_LIST_DIR}/os_prefix.cmake)

set(VERSION "074")
set(LOCAL_ROOT_PATH ${CMAKE_CURRENT_LIST_DIR}/_deps)

set(FILE_PATH ${CMAKE_ARGV3})

set(UNCRUSTIFY_BIN ${LOCAL_ROOT_PATH}/tools/${OS_PREFIX}/uncrustify_${VERSION})

if(NOT EXISTS ${UNCRUSTIFY_BIN})
    include(${CMAKE_CURRENT_LIST_DIR}/download.cmake)
    download_uncrustify(${LOCAL_ROOT_PATH} ${OS_PREFIX} ${VERSION})
endif()

if(NOT EXISTS ${UNCRUSTIFY_BIN})
    message(FATAL_ERROR "Failed to get uncrustify: ${UNCRUSTIFY_BIN}")
endif()

execute_process(
    COMMAND ${UNCRUSTIFY_BIN} -c ${LOCAL_ROOT_PATH}/uncrustify_muse.cfg --no-backup -l CPP ${FILE_PATH}
    RESULT_VARIABLE UNCRUSTIFY_OUT
)

if (UNCRUSTIFY_OUT)
    message(FATAL_ERROR"Failed formatting file: ${FILE_PATH}, error code: ${UNCRUSTIFY_OUT}")
endif()
