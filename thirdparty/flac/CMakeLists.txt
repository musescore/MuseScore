set(MODULE flac)

include(GetPlatformInfo)
include(GetCompilerInfo)
include(GetBuildType)

set(FLAC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/flac-1.3.4)

list(APPEND FLAC_DEFS
    -DHAVE_CONFIG_H)

if(CC_IS_MSVC)
    list(APPEND FLAC_DEFS
        -D_CRT_SECURE_NO_WARNINGS
        -D_USE_MATH_DEFINES)
endif()

if(CMAKE_SYSTEM_PROCESSOR STREQUAL "i686" AND HAVE_STACKREALIGN_FLAG)
    add_compile_options(-mstackrealign)
endif()

if(BUILD_IS_DEBUG)
    list(APPEND FLAC_DEFS
        -DFLAC__OVERFLOW_DETECT)
endif()

aux_source_directory(${FLAC_DIR}/src SOURCE_LIB)

configure_file(${FLAC_DIR}/config.cmake.h.in ${FLAC_DIR}/config.h)

set(MODULE_DEF ${FLAC_DEFS})

set (MODULE_INCLUDE
    ${FLAC_DIR}/include
    ${FLAC_DIR}/src
    ${FLAC_DIR}/src/libFLAC/include
    ${CMAKE_CURRENT_BINARY_DIR}
    )

set (MODULE_SRC
    ${SOURCE_LIB}
    )

set(MODULE_USE_PCH_NONE ON)
set(MODULE_USE_UNITY_NONE ON)
include(${PROJECT_SOURCE_DIR}/build/module.cmake)

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
target_no_warning(${MODULE} -WMSVC-no-translation-unit-is-empty)
target_no_warning(${MODULE} -WMSVC-no-nonstandard-extension-used)
target_no_warning(${MODULE} -WMSVC-no-assignment-within-conditional-expression)
target_no_warning(${MODULE} -WMSVC-no-hides-previous)
target_no_warning(${MODULE} -WMSVC-no-undefined-assuming-extern)
target_no_warning(${MODULE} -WMSVC-named-type-definition-in-parentheses)
