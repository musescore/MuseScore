declare_thirdparty_module(lame)

include(GetPlatformInfo)
include(GetCompilerInfo)

set(LAME_DIR ${CMAKE_CURRENT_SOURCE_DIR})

list(APPEND LAME_DEFS
    -DHAVE_STDINT_H
    -DHAVE_MPGLIB
    -DDECODE_ON_THE_FLY
    -DUSE_FAST_LOG
    -DTAKEHIRO_IEEE754_HACK)

if(CC_IS_MSVC)
    list(APPEND LAME_DEFS
        -DUSE_LAYER_2
        -DHAVE_CONFIG_H
        -D_CRT_SECURE_NO_WARNINGS
        -D_CRT_SECURE_NO_DEPRECATED
        -DTAKEHIRO_IEEE754_HACK)

    configure_file(${LAME_DIR}/configMS.h ${CMAKE_CURRENT_BINARY_DIR}/config.h @ONLY)
else()
    list(APPEND LAME_DEFS
        -DSTDC_HEADERS)
    if(NOT ieee754_float32_t)
        list(APPEND LAME_DEFS
            -Dieee754_float32_t=float)
    endif()
endif()

set(MODULE_DEF
    ${LAME_DEFS})

aux_source_directory(${LAME_DIR}/libmp3lame SOURCE_LIB)
aux_source_directory(${LAME_DIR}/libmp3lame/vector/ SOURCE_LIB)
aux_source_directory(${LAME_DIR}/mpglib SOURCE_LIB)

if(OS_IS_WIN)
    list(APPEND SOURCE_LIB
        ${LAME_DIR}/Dll/BladeMP3EncDLL.c
        ${LAME_DIR}/Dll/BladeMP3EncDLL.h)
endif()

set (MODULE_INCLUDE
    ${LAME_DIR}/include
    ${LAME_DIR}/libmp3lame
    ${LAME_DIR}/mpglib
    ${LAME_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    )

set (MODULE_SRC
    ${SOURCE_LIB}
    )

set(MODULE_USE_UNITY OFF)
set(MODULE_USE_COVERAGE OFF)

setup_module()

#target_no_warning(${MODULE} -Wunused-const-variable=0)
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
target_no_warning(${MODULE} -Wc-no-absolute-value)
target_no_warning(${MODULE} -Wc-no-tautological-pointer-compare)
target_no_warning(${MODULE} -Wc-no-array-parameter)
target_no_warning(${MODULE} -Wno-unused-but-set-variable)
target_no_warning(${MODULE} -WMSVC-no-translation-unit-is-empty)
target_no_warning(${MODULE} -WMSVC-no-nonstandard-extension-used)
target_no_warning(${MODULE} -WMSVC-no-assignment-within-conditional-expression)
target_no_warning(${MODULE} -WMSVC-no-hides-previous)
target_no_warning(${MODULE} -WMSVC-no-undefined-assuming-extern)
target_no_warning(${MODULE} -WMSVC-named-type-definition-in-parentheses)
target_no_warning(${MODULE} -WMSVC-no-benign-redefinition)
