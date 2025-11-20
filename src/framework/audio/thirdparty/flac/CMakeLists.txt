muse_create_thirdparty_module(flac)

include(GetPlatformInfo)
include(GetCompilerInfo)
include(GetBuildType)

set(FLAC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/flac-1.4.3)
set(FLAC_SRC ${FLAC_DIR}/src)

include(GetFlacCompileInfo.cmake)

target_compile_definitions(flac
    PRIVATE HAVE_CONFIG_H
    PUBLIC FLAC__NO_DLL
)

if(CMAKE_SYSTEM_PROCESSOR STREQUAL "i686" AND HAVE_STACKREALIGN_FLAG)
    add_compile_options(-mstackrealign)
endif()

configure_file(${FLAC_DIR}/config.cmake.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)

target_include_directories(flac 
    PUBLIC
        ${FLAC_DIR}/include
        ${FLAC_DIR}/src/libFLAC/include
        ${FLAC_DIR}/src/libFLAC++/include
    PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}
)

target_sources(flac PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}/config.h
    ${FLAC_SRC}/libFLAC/bitmath.c
    ${FLAC_SRC}/libFLAC/bitreader.c
    ${FLAC_SRC}/libFLAC/bitwriter.c
    ${FLAC_SRC}/libFLAC/cpu.c
    ${FLAC_SRC}/libFLAC/crc.c
    ${FLAC_SRC}/libFLAC/fixed.c
    ${FLAC_SRC}/libFLAC/fixed_intrin_avx2.c
    ${FLAC_SRC}/libFLAC/fixed_intrin_sse2.c
    ${FLAC_SRC}/libFLAC/fixed_intrin_sse42.c
    ${FLAC_SRC}/libFLAC/fixed_intrin_ssse3.c
    ${FLAC_SRC}/libFLAC/float.c
    ${FLAC_SRC}/libFLAC/format.c
    ${FLAC_SRC}/libFLAC/lpc.c
    ${FLAC_SRC}/libFLAC/lpc_intrin_avx2.c
    ${FLAC_SRC}/libFLAC/lpc_intrin_fma.c
    ${FLAC_SRC}/libFLAC/lpc_intrin_neon.c
    ${FLAC_SRC}/libFLAC/lpc_intrin_sse2.c
    ${FLAC_SRC}/libFLAC/lpc_intrin_sse41.c
    ${FLAC_SRC}/libFLAC/md5.c
    ${FLAC_SRC}/libFLAC/memory.c
    ${FLAC_SRC}/libFLAC/metadata_iterators.c
    ${FLAC_SRC}/libFLAC/metadata_object.c
    ${FLAC_SRC}/libFLAC/stream_decoder.c
    ${FLAC_SRC}/libFLAC/stream_encoder.c
    ${FLAC_SRC}/libFLAC/stream_encoder_framing.c
    ${FLAC_SRC}/libFLAC/stream_encoder_intrin_avx2.c
    ${FLAC_SRC}/libFLAC/stream_encoder_intrin_sse2.c
    ${FLAC_SRC}/libFLAC/stream_encoder_intrin_ssse3.c
    ${FLAC_SRC}/libFLAC/window.c
    ${FLAC_SRC}/libFLAC++/metadata.cpp
    ${FLAC_SRC}/libFLAC++/stream_decoder.cpp
    ${FLAC_SRC}/libFLAC++/stream_encoder.cpp
    $<$<BOOL:${OS_IS_WIN}>:${FLAC_DIR}/include/share/win_utf8_io.h>
    $<$<BOOL:${OS_IS_WIN}>:${FLAC_SRC}/share/win_utf8_io/win_utf8_io.c>
)

target_no_warning(flac -Wno-deprecated)
target_no_warning(flac -Wno-conversion)
target_no_warning(flac -Wno-unused-parameter)
target_no_warning(flac -Wno-unused-variable)
target_no_warning(flac -Wno-type-limits)
target_no_warning(flac -Wno-unknown-pragmas)
target_no_warning(flac -Wno-conversion)
target_no_warning(flac -Wno-uninitialized)
target_no_warning(flac -Wno-shift-negative-value)
target_no_warning(flac -Wno-implicit-fallthrough)
target_no_warning(flac -Wno-empty-body)
target_no_warning(flac -Wno-attributes)
target_no_warning(flac -WMSVC-no-translation-unit-is-empty)
target_no_warning(flac -WMSVC-no-nonstandard-extension-used)
target_no_warning(flac -WMSVC-no-assignment-within-conditional-expression)
target_no_warning(flac -WMSVC-no-hides-previous)
target_no_warning(flac -WMSVC-no-undefined-assuming-extern)
target_no_warning(flac -WMSVC-named-type-definition-in-parentheses)
target_no_warning(flac -WMSVC-no-unreachable)
