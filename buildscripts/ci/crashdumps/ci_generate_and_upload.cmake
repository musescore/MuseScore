set(HERE ${CMAKE_CURRENT_LIST_DIR})

# Options for generate
set(APP_BIN "" CACHE STRING "Path to app binary")
set(ARCH "" CACHE STRING "System architecture")
set(GENERATE_ARCHS "" CACHE STRING "Generate symbols for architectures")
set(BUILD_DIR "${CMAKE_SOURCE_DIR}/build.release" CACHE STRING "Path to build directory")

set(CONFIG
    -DAPP_BIN=${APP_BIN}
    -DARCH=${ARCH}
    -DGENERATE_ARCHS=${GENERATE_ARCHS}
    -DBUILD_DIR=${BUILD_DIR}
)

execute_process(
    COMMAND cmake ${CONFIG} -P ${HERE}/ci_generate_dumpsyms.cmake
)

# Options for upload
set(SENTRY_URL "" CACHE STRING "Sentry URL")
set(SENTRY_AUTH_TOKEN "" CACHE STRING "Sentry Auth Token")
set(SENTRY_ORG "" CACHE STRING "Sentry Organization")
set(SENTRY_PROJECT "" CACHE STRING "Sentry Project")

set(CONFIG
    -DSENTRY_URL=${SENTRY_URL}
    -DSENTRY_AUTH_TOKEN=${SENTRY_AUTH_TOKEN}
    -DSENTRY_ORG=${SENTRY_ORG}
    -DSENTRY_PROJECT=${SENTRY_PROJECT}
)

execute_process(
    COMMAND cmake ${CONFIG} -P ${HERE}/ci_sentry_dumpsyms_upload.cmake
)
