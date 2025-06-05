declare_thirdparty_module(opusenc)

include(GetPlatformInfo)
include(GetCompilerInfo)
include(GetBuildType)

set(OPUSENC_VERSION 0.2.1)
set(OPUSENC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libopusenc-${OPUSENC_VERSION})

# libopusenc dependent on libopus
include(cmake/SetupOpus.cmake)

aux_source_directory(${OPUSENC_DIR}/src SOURCE_LIB)

configure_file(${OPUSENC_DIR}/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)

set(MODULE_DEF
    -DOUTSIDE_SPEEX
    -DRANDOM_PREFIX=opusenc_prefix)

set(MODULE_INCLUDE
    ${OPUSENC_DIR}/include
    ${OPUSENC_DIR}/src
    ${OPUSENC_DIR}
    ${OPUS_INCLUDE_DIRS}
    )

set(MODULE_SRC
    ${SOURCE_LIB}
    )

set(MODULE_LINK ${OPUS_LIBRARIES})

set(MODULE_USE_UNITY OFF)
set(MODULE_USE_COVERAGE OFF)

setup_module()

target_no_warning(${MODULE} -Wno-unused-parameter)
target_no_warning(${MODULE} -Wno-unused-variable)
target_no_warning(${MODULE} -Wno-type-limits)
target_no_warning(${MODULE} -Wno-unknown-pragmas)
target_no_warning(${MODULE} -Wno-conversion)
target_no_warning(${MODULE} -Wno-uninitialized)
target_no_warning(${MODULE} -Wno-shift-negative-value)
target_no_warning(${MODULE} -Wno-implicit-fallthrough)
target_no_warning(${MODULE} -Wno-empty-body)
target_no_warning(${MODULE} -Wno-attributes)
target_no_warning(${MODULE} -Wno-sign-compare)
target_no_warning(${MODULE} -Wno-restrict)
target_no_warning(${MODULE} -WMSVC-no-translation-unit-is-empty)
target_no_warning(${MODULE} -WMSVC-no-nonstandard-extension-used)
target_no_warning(${MODULE} -WMSVC-no-assignment-within-conditional-expression)
target_no_warning(${MODULE} -WMSVC-no-hides-previous)
target_no_warning(${MODULE} -WMSVC-no-undefined-assuming-extern)
target_no_warning(${MODULE} -WMSVC-named-type-definition-in-parentheses)
