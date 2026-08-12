set(HERE ${CMAKE_CURRENT_LIST_DIR})

set(ARTIFACTS_DIR "build.artifacts")
set(SYMBOLS_PATH "${ARTIFACTS_DIR}/symbols")

set(SENTRY_DOWNLOAD_SCRIPT "https://sentry.io/get-cli")  # Doesn't work on Windows
set(SENTRY_DOWNLOAD_Windows_x86_64 "https://downloads.sentry-cdn.com/sentry-cli/1.59.0/sentry-cli-Windows-x86_64.exe")

set(SENTRY_URL "" CACHE STRING "Sentry URL")
set(SENTRY_AUTH_TOKEN "" CACHE STRING "Sentry Auth Token")
set(SENTRY_ORG "" CACHE STRING "Sentry Organization")
set(SENTRY_PROJECT "" CACHE STRING "Sentry Project")
set(STAGE "" CACHE STRING "Build stage (e.g. stable, testing, nightly, devel)")

# Check
if(NOT SYMBOLS_PATH)
    message(FATAL_ERROR "error: not set SYMBOLS_PATH")
endif()
if(NOT SENTRY_URL)
    message(FATAL_ERROR "error: not set SENTRY_URL")
endif()
if(NOT SENTRY_AUTH_TOKEN)
    message(FATAL_ERROR "error: not set SENTRY_AUTH_TOKEN")
endif()
if(NOT SENTRY_ORG)
    message(FATAL_ERROR "error: not set SENTRY_ORG")
endif()
if(NOT SENTRY_PROJECT)
    message(FATAL_ERROR "error: not set SENTRY_PROJECT")
endif()

# Print
message(STATUS "SYMBOLS_PATH: ${SYMBOLS_PATH}")
message(STATUS "SENTRY_URL: ${SENTRY_URL}")
message(STATUS "SENTRY_AUTH_TOKEN: ${SENTRY_AUTH_TOKEN}")
message(STATUS "SENTRY_ORG: ${SENTRY_ORG}")
message(STATUS "SENTRY_PROJECT: ${SENTRY_PROJECT}")
message(STATUS "STAGE: ${STAGE}")

# Upload symbols
set(ENV{SENTRY_URL} ${SENTRY_URL})
set(ENV{SENTRY_AUTH_TOKEN} ${SENTRY_AUTH_TOKEN})

if(WIN32)
    set(INSTALL_PATH "C:/sentry")
    set(SENTRY_CLI "${INSTALL_PATH}/sentry-cli")
    message(STATUS "windows")

    file(MAKE_DIRECTORY ${INSTALL_PATH})
    file(DOWNLOAD ${SENTRY_DOWNLOAD_Windows_x86_64} ${SENTRY_CLI})
else()
    set(SENTRY_CLI_INSTALL_SCRIPT "sentry-cli.sh")
    file(DOWNLOAD ${SENTRY_DOWNLOAD_SCRIPT} ${SENTRY_CLI_INSTALL_SCRIPT})
    execute_process(COMMAND bash ${SENTRY_CLI_INSTALL_SCRIPT})

    set(SENTRY_CLI "sentry-cli")
endif()

execute_process(
    COMMAND ${SENTRY_CLI} upload-dif -o ${SENTRY_ORG} -p ${SENTRY_PROJECT} ${SYMBOLS_PATH}
    RESULT_VARIABLE result
    OUTPUT_VARIABLE output
    ERROR_VARIABLE output
    ECHO_OUTPUT_VARIABLE
    ECHO_ERROR_VARIABLE
)

if(NOT result EQUAL 0)
    message(FATAL_ERROR "Failed symbols uploaded, code: ${result}")
endif()

# sentry-cli exits 0 even when nothing was uploaded — detect that explicitly
if(STAGE STREQUAL "stable" AND output MATCHES "No debug information files found")
    message(FATAL_ERROR "Failed symbols uploaded: no debug information files found in ${SYMBOLS_PATH}")
endif()

message(STATUS "Success symbols uploaded")
