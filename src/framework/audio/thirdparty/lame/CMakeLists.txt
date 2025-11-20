muse_create_thirdparty_module(lame)

include(GetPlatformInfo)
include(GetCompilerInfo)

set(LAME_DIR ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_definitions(lame PRIVATE
    HAVE_STDINT_H
    HAVE_MPGLIB
    DECODE_ON_THE_FLY
    USE_FAST_LOG
    TAKEHIRO_IEEE754_HACK
)

if(CC_IS_MSVC)
    target_compile_definitions(lame PRIVATE
        USE_LAYER_2
        HAVE_CONFIG_H
        _CRT_SECURE_NO_WARNINGS
        _CRT_SECURE_NO_DEPRECATED
    )

    configure_file(${LAME_DIR}/configMS.h ${CMAKE_CURRENT_BINARY_DIR}/config.h @ONLY)
else()
    target_compile_definitions(lame PRIVATE STDC_HEADERS)
    if(NOT ieee754_float32_t)
        target_compile_definitions(lame PRIVATE
            ieee754_float32_t=float
        )
    endif()
endif()

aux_source_directory(${LAME_DIR}/libmp3lame SOURCE_LIB)
aux_source_directory(${LAME_DIR}/libmp3lame/vector/ SOURCE_LIB)
aux_source_directory(${LAME_DIR}/mpglib SOURCE_LIB)

if(OS_IS_WIN)
    list(APPEND SOURCE_LIB
        ${LAME_DIR}/Dll/BladeMP3EncDLL.c
        ${LAME_DIR}/Dll/BladeMP3EncDLL.h)
endif()

target_sources(lame PRIVATE ${SOURCE_LIB})

target_include_directories(lame 
    PUBLIC
        ${LAME_DIR}/include
    PRIVATE
        ${LAME_DIR}/libmp3lame
        ${LAME_DIR}/mpglib
        ${LAME_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}
    )

#target_no_warning(lame -Wunused-const-variable=0)
target_no_warning(lame -Wno-unused-parameter)
target_no_warning(lame -Wno-unused-variable)
target_no_warning(lame -Wno-type-limits)
target_no_warning(lame -Wno-unknown-pragmas)
target_no_warning(lame -Wno-conversion)
target_no_warning(lame -Wno-uninitialized)
target_no_warning(lame -Wno-shift-negative-value)
target_no_warning(lame -Wno-implicit-fallthrough)
target_no_warning(lame -Wno-empty-body)
target_no_warning(lame -Wno-attributes)
target_no_warning(lame -Wc-no-absolute-value)
target_no_warning(lame -Wc-no-tautological-pointer-compare)
target_no_warning(lame -Wc-no-array-parameter)
target_no_warning(lame -Wno-unused-but-set-variable)
target_no_warning(lame -WMSVC-no-translation-unit-is-empty)
target_no_warning(lame -WMSVC-no-nonstandard-extension-used)
target_no_warning(lame -WMSVC-no-assignment-within-conditional-expression)
target_no_warning(lame -WMSVC-no-hides-previous)
target_no_warning(lame -WMSVC-no-undefined-assuming-extern)
target_no_warning(lame -WMSVC-named-type-definition-in-parentheses)
target_no_warning(lame -WMSVC-no-benign-redefinition)
