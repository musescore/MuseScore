
declare_thirdparty_module(asiosdk)

if(MUSE_MODULE_AUDIO_ASIO_SDK_PATH)
    set(ASIO_SDK_PATH ${MUSE_MODULE_AUDIO_ASIO_SDK_PATH})
else()

    # If not set MUSE_MODULE_AUDIO_ASIO_SDK_PATH
    # download asiosdk source

    set(REMOTE_ROOT_URL https://raw.githubusercontent.com/musescore/muse_deps/main)
    set(remote_url "${REMOTE_ROOT_URL}/asiosdk/ASIO-SDK_2.3.4_2025-10-15")
    set(local_path ${PROJECT_BINARY_DIR}/_deps/asiosdk)
    if (NOT EXISTS ${local_path}/asiosdk.cmake)
        file(MAKE_DIRECTORY ${local_path})
        file(DOWNLOAD ${remote_url}/asiosdk.cmake ${local_path}/asiosdk.cmake
            HTTPHEADER "Cache-Control: no-cache"
        )
    endif()

    include(${local_path}/asiosdk.cmake)

    # func from ${name}.cmake)
    cmake_language(CALL asiosdk_Populate ${remote_url} ${local_path} "source" "" "")

    set(ASIO_SDK_PATH ${local_path})
    message(STATUS "ASIO_SDK_PATH: ${ASIO_SDK_PATH}")

endif()

set(ASIO_SDK_SRC
    ${ASIO_SDK_PATH}/ASIOSDK/common/asio.cpp
    ${ASIO_SDK_PATH}/ASIOSDK/common/asio.h
    ${ASIO_SDK_PATH}/ASIOSDK/host/asiodrivers.cpp
    ${ASIO_SDK_PATH}/ASIOSDK/host/asiodrivers.h
    ${ASIO_SDK_PATH}/ASIOSDK/host/pc/asiolist.cpp
    ${ASIO_SDK_PATH}/ASIOSDK/host/pc/asiolist.h
)

set(MODULE_SRC
    ${ASIO_SDK_SRC}
    )

set(MODULE_INCLUDE
    ${ASIO_SDK_PATH}
    ${ASIO_SDK_PATH}/ASIOSDK/common
    ${ASIO_SDK_PATH}/ASIOSDK/host
    ${ASIO_SDK_PATH}/ASIOSDK/host/pc
    )

set(MODULE_DEF
    -DIEEE754_64FLOAT
    -DNATIVE_INT64
)

set(MODULE_USE_UNITY OFF)

# ASIO SDK is written without UNICODE support (it uses variables like char*).
# Define UNICODE is returned at the end of adding the audio module.
remove_definitions(-DUNICODE)

setup_module()

