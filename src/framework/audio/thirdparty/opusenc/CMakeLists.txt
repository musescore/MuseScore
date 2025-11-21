muse_create_thirdparty_module(opusenc)

include(GetPlatformInfo)
include(GetCompilerInfo)
include(GetBuildType)

set(OPUSENC_VERSION 0.2.1)
set(OPUSENC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libopusenc-${OPUSENC_VERSION})

# libopusenc dependent on libopus
include(cmake/SetupOpus.cmake)

aux_source_directory(${OPUSENC_DIR}/src SOURCE_LIB)

configure_file(${OPUSENC_DIR}/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)

target_sources(opusenc PRIVATE
    ${SOURCE_LIB}
    ${CMAKE_CURRENT_BINARY_DIR}/config.h
)

target_compile_definitions(opusenc PRIVATE
    OUTSIDE_SPEEX
    RANDOM_PREFIX=opusenc_prefix
)

target_include_directories(opusenc 
    PUBLIC
        ${OPUSENC_DIR}/include
        ${OPUS_INCLUDE_DIRS}
    PRIVATE
        ${OPUSENC_DIR}/src
        ${OPUSENC_DIR}
)

target_link_libraries(opusenc PRIVATE ${OPUS_LIBRARIES})

target_no_warning(opusenc -Wno-unused-parameter)
target_no_warning(opusenc -Wno-unused-variable)
target_no_warning(opusenc -Wno-type-limits)
target_no_warning(opusenc -Wno-unknown-pragmas)
target_no_warning(opusenc -Wno-conversion)
target_no_warning(opusenc -Wno-uninitialized)
target_no_warning(opusenc -Wno-shift-negative-value)
target_no_warning(opusenc -Wno-implicit-fallthrough)
target_no_warning(opusenc -Wno-empty-body)
target_no_warning(opusenc -Wno-attributes)
target_no_warning(opusenc -Wno-sign-compare)
target_no_warning(opusenc -Wno-restrict)
target_no_warning(opusenc -WMSVC-no-translation-unit-is-empty)
target_no_warning(opusenc -WMSVC-no-nonstandard-extension-used)
target_no_warning(opusenc -WMSVC-no-assignment-within-conditional-expression)
target_no_warning(opusenc -WMSVC-no-hides-previous)
target_no_warning(opusenc -WMSVC-no-undefined-assuming-extern)
target_no_warning(opusenc -WMSVC-named-type-definition-in-parentheses)
