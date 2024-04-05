# 3.1 is OK for most parts. However:
# 3.3 is needed in src/libFLAC
# 3.5 is needed in src/libFLAC/ia32
# 3.9 is needed in 'doc' because of doxygen_add_docs()
cmake_minimum_required(VERSION 3.5)

if(NOT (CMAKE_BUILD_TYPE OR CMAKE_CONFIGURATION_TYPES OR DEFINED ENV{CFLAGS} OR DEFINED ENV{CXXFLAGS}))
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build, options are: None Debug Release RelWithDebInfo")
endif()

project(FLAC VERSION 1.4.3) # HOMEPAGE_URL "https://www.xiph.org/flac/")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

option(BUILD_CXXLIBS "Build libFLAC++" ON)
option(BUILD_PROGRAMS "Build and install programs" ON)
option(BUILD_EXAMPLES "Build and install examples" ON)
option(BUILD_TESTING "Build tests" ON)
option(BUILD_DOCS "Build and install doxygen documents" ON)
option(WITH_FORTIFY_SOURCE "Enable protection against buffer overflows" ON)
option(WITH_STACK_PROTECTOR "Enable GNU GCC stack smash protection" ON)
option(INSTALL_MANPAGES "Install MAN pages" ON)
option(INSTALL_PKGCONFIG_MODULES "Install PkgConfig modules" ON)
option(INSTALL_CMAKE_CONFIG_MODULE "Install CMake package-config module" ON)
option(WITH_OGG "ogg support (default: test for libogg)" ON)
option(BUILD_SHARED_LIBS "Build shared instead of static libraries" OFF)

set(VERSION ${PROJECT_VERSION})

if(NOT UNIX)
    # This is to make sure testing works when building with a DLL
    set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/objs)
    set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/objs)
endif()

if(WITH_OGG)
    if(IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/ogg")
        add_subdirectory("ogg")
        set(OGG_FOUND 1 CACHE INTERNAL "ogg has been added as subdirectory")
        set_target_properties(ogg PROPERTIES FOLDER Libraries)
        if(BUILD_TESTING)
            set_target_properties(test_bitwise test_framing PROPERTIES FOLDER Tests)
        endif()
    else()
        if(NOT TARGET Ogg::ogg)
            find_package(Ogg REQUIRED)
        else()
            set(OGG_FOUND 1 CACHE INTERNAL "ogg has already been built")
        endif()
        set(OGG_PACKAGE "ogg")
    endif()
endif()

find_program (HAVE_GIT git)

if(HAVE_GIT)
    execute_process(
        COMMAND git --git-dir=.git describe --tags --exact-match
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_COMMIT_TAG
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
        )
    execute_process(
        COMMAND git --git-dir=.git log -1 --pretty=format:%h
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_COMMIT_HASH
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
        )
    execute_process(
        COMMAND git --git-dir=.git log -1 --pretty=format:%cd --date=format:%Y%m%d
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_COMMIT_DATE
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
        )
endif()

if(NOT WIN32)
    find_package(Iconv)
    set(HAVE_ICONV ${Iconv_FOUND})
endif()

if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_C_FLAGS "-Wall -Wextra -Wstrict-prototypes -Wmissing-prototypes -Waggregate-return -Wcast-align -Wnested-externs -Wshadow -Wundef -Wmissing-declarations -Winline ${CMAKE_C_FLAGS}")
    set(CMAKE_C_FLAGS_RELEASE "-DNDEBUG ${CMAKE_C_FLAGS_RELEASE}")
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wcast-align -Wshadow -Wwrite-strings -Wctor-dtor-privacy -Wnon-virtual-dtor -Wreorder -Wsign-promo -Wundef ${CMAKE_CXX_FLAGS}")
    set(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG ${CMAKE_CXX_FLAGS_RELEASE}")
endif()
if(MSVC)
    set(CMAKE_C_FLAGS_RELEASE "/O2 /Ob2 /Oi /Ot /Oy /DNDEBUG ${CMAKE_C_FLAGS_RELEASE}")
endif()

include(CMakePackageConfigHelpers)
include(CheckCCompilerFlag)
include(CheckCXXCompilerFlag)
include(CheckSymbolExists)
include(CheckFunctionExists)
include(CheckIncludeFile)
include(CheckCSourceCompiles)
include(CheckCXXSourceCompiles)
include(CheckLibraryExists)
include(GNUInstallDirs)
include(UseSystemExtensions)
include(TestBigEndian)
enable_testing()

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

check_c_compiler_flag(-Werror HAVE_WERROR_FLAG)
check_c_compiler_flag(-Wdeclaration-after-statement HAVE_DECL_AFTER_STMT_FLAG)
check_c_compiler_flag(-mstackrealign HAVE_STACKREALIGN_FLAG)
check_cxx_compiler_flag(-Weffc++ HAVE_WEFFCXX_FLAG)

if(MINGW AND (WITH_FORTIFY_SOURCE OR WITH_STACK_PROTECTOR))
  check_library_exists("ssp.a"  __stack_chk_fail "" HAVE_LIBSSP)
  if(NOT HAVE_LIBSSP)
    message(WARNING "Could not find libssp in MinGW, stack protection and/or FORTIFY_SOURCE are unavailable")
  else()
    link_libraries("ssp.a")
  endif()
elseif(NOT MSVC)
  set(HAVE_LIBSSP 1)
endif()

if(WITH_STACK_PROTECTOR)
  if(NOT MSVC)
    check_c_compiler_flag("-fstack-protector-strong" HAVE_STACK_PROTECTOR_FLAG)
  endif()
endif()

if(HAVE_WERROR_FLAG)
    option(ENABLE_WERROR "Enable -Werror in all Makefiles" OFF)
endif()

add_compile_options(
    $<$<BOOL:${MSVC}>:/wd4267>
    $<$<BOOL:${MSVC}>:/wd4996>
    $<$<BOOL:${ENABLE_WERROR}>:-Werror>
    $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<BOOL:${HAVE_WEFFCXX_FLAG}>>:-Weffc++>
    $<$<AND:$<COMPILE_LANGUAGE:C>,$<BOOL:${HAVE_DECL_AFTER_STMT_FLAG}>>:-Wdeclaration-after-statement>)

if(WITH_FORTIFY_SOURCE AND HAVE_LIBSSP)
  add_definitions(-D_FORTIFY_SOURCE=2)
endif()

if(HAVE_STACK_PROTECTOR_FLAG AND HAVE_LIBSSP)
    add_compile_options($<$<OR:$<COMPILE_LANGUAGE:C>,$<COMPILE_LANGUAGE:CXX>>:-fstack-protector-strong>)
endif()

if(CMAKE_SYSTEM_PROCESSOR STREQUAL "i686" AND HAVE_STACKREALIGN_FLAG)
    add_compile_options($<$<OR:$<COMPILE_LANGUAGE:C>,$<COMPILE_LANGUAGE:CXX>>:-mstackrealign>)
endif()

include_directories("include")

include_directories("${CMAKE_CURRENT_BINARY_DIR}")
add_definitions(-DHAVE_CONFIG_H)

if(MSVC)
    add_definitions(
        -D_CRT_SECURE_NO_WARNINGS
        -D_USE_MATH_DEFINES)
endif()
if(CMAKE_BUILD_TYPE STREQUAL Debug OR CMAKE_BUILD_TYPE STREQUAL RelWithDebInfo)
    add_definitions(-DFLAC__OVERFLOW_DETECT)
endif()

add_subdirectory("src")
add_subdirectory("microbench")
if(BUILD_DOCS)
    add_subdirectory("doc")
endif()
if(BUILD_EXAMPLES)
    add_subdirectory("examples")
endif()
if(BUILD_TESTING)
    add_subdirectory("test")
endif()

# The following folder layout is mostly for MSVC
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set_target_properties(FLAC grabbag getopt replaygain_analysis replaygain_synthesis utf8 PROPERTIES FOLDER Libraries)
if(BUILD_CXXLIBS)
    set_target_properties(FLAC++ PROPERTIES FOLDER Libraries)
endif()
if(BUILD_PROGRAMS)
    set_target_properties(flacapp metaflac PROPERTIES FOLDER Programs)
endif()
if(BUILD_TESTING)
    set_target_properties(test_libFLAC test_libs_common test_picture test_seeking test_streams test_cuesheet PROPERTIES FOLDER Tests)
	if(BUILD_CXXLIBS)
	    set_target_properties(test_libFLAC++ PROPERTIES FOLDER Tests)
	endif()
endif()
if(BUILD_EXAMPLES)
    set_target_properties(decode_file encode_file PROPERTIES FOLDER Examples)
    if(BUILD_CXXLIBS)
        set_target_properties(decode_file_cxx encode_file_cxx PROPERTIES FOLDER Examples)
    endif()
endif()
if(BUILD_UTILS)
    set_target_properties(flacdiff flactimer PROPERTIES FOLDER Utils)
endif()

configure_file(config.cmake.h.in config.h)

if(INSTALL_CMAKE_CONFIG_MODULE)
    install(
        EXPORT targets
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
        NAMESPACE FLAC::)
    export(EXPORT targets NAMESPACE FLAC:: FILE FLACTargets.cmake)

    configure_package_config_file(
        ${PROJECT_SOURCE_DIR}/flac-config.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/flac-config.cmake
        INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}")
    write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/flac-config-version.cmake COMPATIBILITY AnyNewerVersion)

    install(
        FILES ${CMAKE_CURRENT_BINARY_DIR}/flac-config.cmake ${CMAKE_CURRENT_BINARY_DIR}/flac-config-version.cmake
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
    )

    install(
        FILES
            "${CMAKE_CURRENT_BINARY_DIR}/flac-config.cmake"
            "${CMAKE_CURRENT_BINARY_DIR}/flac-config-version.cmake"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}")
endif()

file(GLOB FLAC_HEADERS "include/FLAC/*.h")
file(GLOB FLAC++_HEADERS "include/FLAC++/*.h")
install(FILES ${FLAC_HEADERS} DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/FLAC")
install(FILES ${FLAC++_HEADERS} DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/FLAC++")
if(INSTALL_MANPAGES)
    find_program (HAVE_PANDOC pandoc)
    if(HAVE_PANDOC)
        file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/man")
        add_custom_command(
            OUTPUT man/flac.1
            COMMAND pandoc --standalone --to man "${CMAKE_SOURCE_DIR}/man/flac.md" > man/flac.1
            )
        add_custom_command(
            OUTPUT man/metaflac.1
            COMMAND pandoc --standalone --to man "${CMAKE_SOURCE_DIR}/man/metaflac.md" > man/metaflac.1
            )
               add_custom_target(man ALL DEPENDS man/flac.1 man/metaflac.1)
        install(FILES "${CMAKE_BINARY_DIR}/man/flac.1" "${CMAKE_BINARY_DIR}/man/metaflac.1" DESTINATION "${CMAKE_INSTALL_MANDIR}/man1")
    else()
        if(EXISTS "${CMAKE_SOURCE_DIR}/man/flac.1" AND EXISTS "${CMAKE_SOURCE_DIR}/man/metaflac.1")
            install(FILES "man/flac.1" "man/metaflac.1" DESTINATION "${CMAKE_INSTALL_MANDIR}/man1")
        else()
            message(SEND_ERROR "Pandoc nor prebuild manpages are found. Cannot install manpages. Set INSTALL_MANPAGES to OFF to build without man pages")
        endif()
    endif()
endif()
