include(CheckCCompilerFlag)
include(CheckFunctionExists)
include(CheckIncludeFile)
include(CheckCSourceCompiles)
include(TestBigEndian)

include(${FLAC_DIR}/cmake/UseSystemExtensions.cmake)

check_include_file("byteswap.h" HAVE_BYTESWAP_H)
check_include_file("inttypes.h" HAVE_INTTYPES_H)
check_include_file("stdint.h" HAVE_STDINT_H)
check_include_file("stdbool.h" HAVE_STDBOOL_H)
check_include_file("arm_neon.h" FLAC__HAS_NEONINTRIN)

if(NOT HAVE_STDINT_H OR NOT HAVE_STDBOOL_H)
    message(SEND_ERROR "Header stdint.h and/or stdbool.h not found")
endif()

if(MSVC)
    check_include_file("intrin.h" FLAC__HAS_X86INTRIN)
else()
    check_include_file("x86intrin.h" FLAC__HAS_X86INTRIN)
endif()

check_function_exists(fseeko HAVE_FSEEKO)

check_c_source_compiles("int main() { return __builtin_bswap16 (0) ; }" HAVE_BSWAP16)
check_c_source_compiles("int main() { return __builtin_bswap32 (0) ; }" HAVE_BSWAP32)
check_c_source_compiles("
    #include <langinfo.h>
    int main()
    {
        char* cs = nl_langinfo(CODESET);
        return !cs;
    }"
    HAVE_LANGINFO_CODESET)

test_big_endian(CPU_IS_BIG_ENDIAN)

check_c_compiler_flag(-mstackrealign HAVE_STACKREALIGN_FLAG)
