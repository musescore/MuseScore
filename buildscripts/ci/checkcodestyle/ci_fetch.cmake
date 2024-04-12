

set(REMOTE_ROOT_URL https://raw.githubusercontent.com/musescore/MuseScore/master/buildscripts/ci/checkcodestyle)
set(LOCAL_ROOT_PATH ${CMAKE_CURRENT_LIST_DIR}/_deps/checkcodestyle)

file(DOWNLOAD ${REMOTE_ROOT_URL}/ci_files.cmake ${LOCAL_ROOT_PATH}/ci_files.cmake )

include(${LOCAL_ROOT_PATH}/ci_files.cmake)

foreach(FILE_PATH ${CI_FILES})
    file(DOWNLOAD ${REMOTE_ROOT_URL}/${FILE_PATH} ${LOCAL_ROOT_PATH}/${FILE_PATH} )
endforeach()
