option(WITH_ASM "Use any assembly optimization routines" ON)

check_include_file("cpuid.h" HAVE_CPUID_H)
check_include_file("sys/param.h" HAVE_SYS_PARAM_H)

set(CMAKE_REQUIRED_LIBRARIES m)
check_function_exists(lround HAVE_LROUND)

include(CheckCSourceCompiles)
include(CheckCPUArch)

check_cpu_arch_x64(FLAC__CPU_X86_64)
if(NOT FLAC__CPU_X86_64)
    check_cpu_arch_x86(FLAC__CPU_IA32)
endif()

if(FLAC__CPU_X86_64 OR FLAC__CPU_IA32)
    set(FLAC__ALIGN_MALLOC_DATA 1)
    option(WITH_AVX "Enable AVX, AVX2 optimizations" ON)
endif()

include(CheckLanguage)
check_language(ASM_NASM)
if(CMAKE_ASM_NASM_COMPILER)
    enable_language(ASM_NASM)
    add_definitions(-DFLAC__HAS_NASM)
endif()

if(NOT WITH_ASM)
    add_definitions(-DFLAC__NO_ASM)
endif()

if(FLAC__CPU_IA32)
    if(WITH_ASM AND CMAKE_ASM_NASM_COMPILER)
        add_subdirectory(ia32)
    endif()

    option(WITH_SSE "Enable SSE2 optimizations" ON)
    check_c_compiler_flag(-msse2 HAVE_MSSE2_FLAG)
    if(WITH_SSE)
        add_compile_options(
            $<$<BOOL:${HAVE_MSSE2_FLAG}>:-msse2>
            $<$<BOOL:${MSVC}>:/arch:SSE2>)
    endif()
endif()

include_directories("include")

add_library(FLAC
    bitmath.c
    bitreader.c
    bitwriter.c
    cpu.c
    crc.c
    fixed.c
    fixed_intrin_sse2.c
    fixed_intrin_ssse3.c
    float.c
    format.c
    lpc.c
    lpc_intrin_sse.c
    lpc_intrin_sse2.c
    lpc_intrin_sse41.c
    lpc_intrin_avx2.c
    lpc_intrin_vsx.c
    md5.c
    memory.c
    metadata_iterators.c
    metadata_object.c
    stream_decoder.c
    stream_encoder.c
    stream_encoder_intrin_sse2.c
    stream_encoder_intrin_ssse3.c
    stream_encoder_intrin_avx2.c
    stream_encoder_framing.c
    window.c
    $<$<BOOL:${WIN32}>:../../include/share/windows_unicode_filenames.h>
    $<$<BOOL:${WIN32}>:windows_unicode_filenames.c>
    $<$<BOOL:${OGG_FOUND}>:ogg_decoder_aspect.c>
    $<$<BOOL:${OGG_FOUND}>:ogg_encoder_aspect.c>
    $<$<BOOL:${OGG_FOUND}>:ogg_helper.c>
    $<$<BOOL:${OGG_FOUND}>:ogg_mapping.c>)
if(TARGET FLAC-asm)
    target_sources(FLAC PRIVATE $<TARGET_OBJECTS:FLAC-asm>)
endif()

target_compile_definitions(FLAC
    PRIVATE $<$<BOOL:${BUILD_SHARED_LIBS}>:FLAC_API_EXPORTS>
    PUBLIC $<$<NOT:$<BOOL:${BUILD_SHARED_LIBS}>>:FLAC__NO_DLL>)
if(NOT WIN32)
    target_compile_definitions(FLAC PRIVATE $<$<BOOL:${BUILD_SHARED_LIBS}>:FLAC__USE_VISIBILITY_ATTR>)
endif()
target_include_directories(FLAC INTERFACE
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")
target_link_libraries(FLAC PUBLIC $<$<BOOL:${HAVE_LROUND}>:m>)
if(TARGET Ogg::ogg)
    target_link_libraries(FLAC PUBLIC Ogg::ogg)
endif()
if(BUILD_SHARED_LIBS)
    set_target_properties(FLAC PROPERTIES
        VERSION 8.3.0
        SOVERSION 8)
    if(NOT WIN32)
        set_target_properties(FLAC PROPERTIES C_VISIBILITY_PRESET hidden)
    endif()
endif()

add_library(FLAC::FLAC ALIAS FLAC)


install(TARGETS FLAC EXPORT targets
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}/"
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}/"
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}/")

if(INSTALL_PKGCONFIG_MODULES)
    set(prefix "${CMAKE_INSTALL_PREFIX}")
    set(exec_prefix "${CMAKE_INSTALL_PREFIX}")
    set(libdir "${CMAKE_INSTALL_FULL_LIBDIR}")
    set(includedir "${CMAKE_INSTALL_FULL_INCLUDEDIR}")
    configure_file(flac.pc.in flac.pc @ONLY)
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/flac.pc"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig")
endif()
