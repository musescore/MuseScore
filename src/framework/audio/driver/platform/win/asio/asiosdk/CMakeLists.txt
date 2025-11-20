
muse_create_thirdparty_module(asiosdk)

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

target_sources(asiosdk PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/asiosdk_asio.cpp
    ${CMAKE_CURRENT_LIST_DIR}/asiosdk_asiolist.cpp
    ${CMAKE_CURRENT_LIST_DIR}/asiosdk_asiodrivers.cpp
)

target_include_directories(asiosdk PUBLIC
    ${ASIO_SDK_PATH}
    ${ASIO_SDK_PATH}/ASIOSDK/common
    ${ASIO_SDK_PATH}/ASIOSDK/host
    ${ASIO_SDK_PATH}/ASIOSDK/host/pc
)

# Silence warnings from SDK
target_compile_options(asiosdk PRIVATE /wd4267)
