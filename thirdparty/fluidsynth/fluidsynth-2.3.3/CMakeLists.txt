# FluidSynth - A Software Synthesizer
#
# Copyright (C) 2003-2023 Peter Hanappe and others.
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public License
# as published by the Free Software Foundation; either version 2.1 of
# the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
# 02111-1307, USA

# CMake based build system. Pedro Lopez-Cabanillas <plcl@users.sf.net>

cmake_minimum_required ( VERSION 3.13 )
# 3.1 because of CMAKE_C_STANDARD
# 3.11 because COMPATIBILITY SameMinorVersion in CMakePackageConfigHelpers
# 3.13.5 because it is the latest supported in Windows XP

if(POLICY CMP0075) # CMake version 3.13.5 warns when the policy is not set or value is OLD
  cmake_policy(SET CMP0075 NEW)
endif()

if(POLICY CMP0091) # new in CMake 3.15, defaults to OLD
  cmake_policy(SET CMP0091 NEW)
endif()

if(POLICY CMP0099) # new in CMake 3.17, defaults to OLD
  cmake_policy(SET CMP0099 NEW)
elseif(NOT BUILD_SHARED_LIBS)
  message(WARNING "Your version of CMake is very old. This may cause linking issues if your dependencies are not in your compiler's default search paths.")
endif()

project ( FluidSynth C CXX )
list( APPEND CMAKE_MODULE_PATH ${FluidSynth_SOURCE_DIR}/cmake_admin )

# FluidSynth package name
set ( PACKAGE "fluidsynth" )

# FluidSynth package version
set ( FLUIDSYNTH_VERSION_MAJOR 2 )
set ( FLUIDSYNTH_VERSION_MINOR 3 )
set ( FLUIDSYNTH_VERSION_MICRO 3 )
set ( VERSION "${FLUIDSYNTH_VERSION_MAJOR}.${FLUIDSYNTH_VERSION_MINOR}.${FLUIDSYNTH_VERSION_MICRO}" )
set ( FLUIDSYNTH_VERSION "\"${VERSION}\"" )

# libfluidsynth - Library version
# *** NOTICE ***
# Update library version upon each release (follow these steps in order)
# if any source code changes: REVISION++
# if any interfaces added/removed/changed: REVISION=0
# if any interfaces removed/changed (compatibility broken): CURRENT++
# if any interfaces have been added: AGE++
# if any interfaces have been removed/changed (compatibility broken): AGE=0
# This is not exactly the same algorithm as the libtool one, but the results are the same.
set ( LIB_VERSION_CURRENT 3 )
set ( LIB_VERSION_AGE 2 )
set ( LIB_VERSION_REVISION 1 )
set ( LIB_VERSION_INFO
      "${LIB_VERSION_CURRENT}.${LIB_VERSION_AGE}.${LIB_VERSION_REVISION}" )

# Options disabled by default
option ( enable-coverage "enable gcov code coverage" off )
option ( enable-floats "enable type float instead of double for DSP samples" off )
option ( enable-fpe-check "enable Floating Point Exception checks and debug messages" off )
option ( enable-portaudio "compile PortAudio support" off )
option ( enable-profiling "profile the dsp code" off )
option ( enable-trap-on-fpe "enable SIGFPE trap on Floating Point Exceptions" off )
option ( enable-ubsan "compile and link against UBSan (for debugging fluidsynth internals)" off )

# Options enabled by default
option ( enable-aufile "compile support for sound file output" on )
option ( BUILD_SHARED_LIBS "Build a shared object or DLL" on )
option ( enable-dbus "compile DBUS support (if it is available)" on )
option ( enable-ipv6  "enable IPv6 support at the cost of disabling IPv4" on )
option ( enable-jack "compile JACK support (if it is available)" on )
option ( enable-ladspa "enable LADSPA effect units" on )
option ( enable-libinstpatch "use libinstpatch (if available) to load DLS and GIG files" on )
option ( enable-libsndfile "compile libsndfile support (if it is available)" on )
option ( enable-midishare "compile MidiShare support (if it is available)" on )
option ( enable-opensles "compile OpenSLES support (if it is available)" off )
option ( enable-oboe "compile Oboe support (requires OpenSLES and/or AAudio)" off )
option ( enable-network "enable network support (requires BSD sockets)" on )
option ( enable-oss "compile OSS support (if it is available)" on )
option ( enable-dsound "compile DirectSound support (if it is available)" on )
option ( enable-wasapi "compile Windows WASAPI support (if it is available)" on )
option ( enable-waveout "compile Windows WaveOut support (if it is available)" on )
option ( enable-winmidi "compile Windows MIDI support (if it is available)" on )
option ( enable-sdl2 "compile SDL2 audio support (if it is available)" on )
option ( enable-pulseaudio "compile PulseAudio support (if it is available)" on )
option ( enable-pipewire "compile PipeWire support (if it is available)" on )
option ( enable-readline "compile readline lib line editing (if it is available)" on )
option ( enable-threads "enable multi-threading support (such as parallel voice synthesis)" on )
option ( enable-openmp "enable OpenMP support (parallelization of soundfont decoding, vectorization of voice mixing, etc.)" on )

# Platform specific options
if ( CMAKE_SYSTEM MATCHES "Linux|FreeBSD|DragonFly" )
    option ( enable-lash "compile LASH support (if it is available)" on )
    option ( enable-alsa "compile ALSA support (if it is available)" on )
endif ( CMAKE_SYSTEM MATCHES "Linux|FreeBSD|DragonFly" )

if ( CMAKE_SYSTEM MATCHES "Linux" )
    option ( enable-systemd "compile systemd support (if it is available)" on )
endif ( CMAKE_SYSTEM MATCHES "Linux" )

if ( CMAKE_SYSTEM MATCHES "Darwin" )
    option ( enable-coreaudio "compile CoreAudio support (if it is available)" on )
    option ( enable-coremidi "compile CoreMIDI support (if it is available)" on )
    option ( enable-framework "create a Mac OSX style FluidSynth.framework" on )
endif ( CMAKE_SYSTEM MATCHES "Darwin" )

if ( CMAKE_SYSTEM MATCHES "OS2" )
    option ( enable-dart "compile DART support (if it is available)" on )
    set ( enable-ipv6 off )
endif ( CMAKE_SYSTEM MATCHES "OS2" )

# the default C standard to use for all targets
set(CMAKE_C_STANDARD 90)

# the default C++ standard to use for all targets
set(CMAKE_CXX_STANDARD 98)

# whether to use gnu extensions
set(CMAKE_C_EXTENSIONS ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compile with position independent code if the user requested a shared lib, i.e. no PIC if static requested.
# This is cmakes default behavior, but here it's explicitly required due to the use of libfluidsynth-OBJ as object library,
# which would otherwise always be compiled without PIC.
if ( NOT CMAKE_POSITION_INDEPENDENT_CODE )
    set ( CMAKE_POSITION_INDEPENDENT_CODE ${BUILD_SHARED_LIBS} )
endif ( NOT CMAKE_POSITION_INDEPENDENT_CODE )

# the default global visibility level for all target
# no visibility support on OS2
if ( NOT OS2 )
    set ( CMAKE_C_VISIBILITY_PRESET hidden )
endif ( NOT OS2 )

# enforce visibility control for all types of cmake targets
if ( POLICY CMP0063 ) # since version 3.3, CMake version 3.21.2 warns when the policy is not set and uses OLD behavior.
  cmake_policy ( SET CMP0063 NEW )
endif ( POLICY CMP0063 )

# Default install directory names, some provided by GNUInstallDirs
include ( DefaultDirs )

# Basic C library checks
include ( CheckCCompilerFlag )
include ( CheckSTDC )
include ( CheckIncludeFile )
include ( CheckSymbolExists )
include ( CheckTypeSize )
check_include_file ( string.h HAVE_STRING_H )
check_include_file ( strings.h HAVE_STRINGS_H )
check_include_file ( stdlib.h HAVE_STDLIB_H )
check_include_file ( stdio.h HAVE_STDIO_H )
check_include_file ( math.h HAVE_MATH_H )
check_include_file ( errno.h HAVE_ERRNO_H )
check_include_file ( stdarg.h HAVE_STDARG_H )
check_include_file ( unistd.h HAVE_UNISTD_H )
check_include_file ( sys/mman.h HAVE_SYS_MMAN_H )
check_include_file ( sys/types.h HAVE_SYS_TYPES_H )
check_include_file ( sys/time.h HAVE_SYS_TIME_H )
check_include_file ( sys/stat.h HAVE_SYS_STAT_H )
check_include_file ( fcntl.h HAVE_FCNTL_H )
check_include_file ( sys/socket.h HAVE_SYS_SOCKET_H )
check_include_file ( netinet/in.h HAVE_NETINET_IN_H )
check_include_file ( netinet/tcp.h HAVE_NETINET_TCP_H )
check_include_file ( arpa/inet.h HAVE_ARPA_INET_H )
check_include_file ( limits.h  HAVE_LIMITS_H )
check_include_file ( pthread.h HAVE_PTHREAD_H )
check_include_file ( signal.h HAVE_SIGNAL_H )
check_include_file ( getopt.h HAVE_GETOPT_H )
check_include_file ( stdint.h HAVE_STDINT_H )
check_type_size ( "long long" LONG_LONG )
if ( NOT HAVE_LONG_LONG AND NOT MSVC)
    message ( FATAL_ERROR "Your compiler does not support intrinsic type 'long long'. Unable to compile fluidsynth." )
endif ()

include ( CMakePrintHelpers ) # for cmake_print_properties() and cmake_print_variables() 
include ( TestInline )
include ( TestVLA )
include ( TestBigEndian )
test_big_endian ( WORDS_BIGENDIAN )

unset ( LIBFLUID_CPPFLAGS CACHE )
unset ( LIBFLUID_LIBS CACHE )
unset ( FLUID_CPPFLAGS CACHE )
unset ( FLUID_LIBS CACHE )
unset ( ENABLE_UBSAN CACHE )

if ( CMAKE_COMPILER_IS_GNUCC OR CMAKE_C_COMPILER_ID MATCHES "Clang" OR CMAKE_C_COMPILER_ID STREQUAL "Intel" )
  if ( NOT APPLE AND NOT OS2 AND NOT EMSCRIPTEN )
    set ( CMAKE_EXE_LINKER_FLAGS
          "${CMAKE_EXE_LINKER_FLAGS} -Wl,--as-needed" )
    set ( CMAKE_SHARED_LINKER_FLAGS
          "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-undefined" )
  endif ( NOT APPLE AND NOT OS2 AND NOT EMSCRIPTEN )

  # define some warning flags
  set ( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -W -Wpointer-arith -Wcast-qual -Wstrict-prototypes -Wno-unused-parameter -Wdeclaration-after-statement -Werror=implicit-function-declaration" )
  check_c_compiler_flag ( "-Werror=incompatible-pointer-types" HAVE_INCOMPATIBLE_POINTER_TYPES )
  if ( HAVE_INCOMPATIBLE_POINTER_TYPES )
    set ( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror=incompatible-pointer-types" )
  endif ( HAVE_INCOMPATIBLE_POINTER_TYPES )

  set ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -W -Wpointer-arith -Wcast-qual -Wno-unused-parameter" )

  # prepend to build type specific flags, to allow users to override
  set ( CMAKE_C_FLAGS_DEBUG "-g ${CMAKE_C_FLAGS_DEBUG}" )

  if ( CMAKE_C_COMPILER_ID STREQUAL "Intel" )
    # icc needs the restrict flag to recognize C99 restrict pointers
    set ( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -restrict" )
  else () # not intel
    # gcc and clang support bad function cast and alignment warnings; add them as well.
    set ( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wbad-function-cast -Wcast-align" )
    set ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wcast-align" )

    if ( enable-ubsan )
      set ( CMAKE_C_FLAGS "-fsanitize=address,undefined ${CMAKE_C_FLAGS}" )
      set ( CMAKE_CXX_FLAGS "-fsanitize=address,undefined ${CMAKE_CXX_FLAGS}" )
      set ( CMAKE_EXE_LINKER_FLAGS "-fsanitize=address,undefined ${CMAKE_EXE_LINKER_FLAGS}" )
      set ( CMAKE_SHARED_LINKER_FLAGS "-fsanitize=address,undefined ${CMAKE_SHARED_LINKER_FLAGS}" )
      set ( ENABLE_UBSAN 1 )
    endif ( enable-ubsan )

    if ( enable-coverage )
      if ( CMAKE_COMPILER_IS_GNUCXX )
        include ( CodeCoverage )
        set ( CODE_COVERAGE_VERBOSE TRUE )

        append_coverage_compiler_flags()
        setup_target_for_coverage_gcovr_html (
            NAME coverage
            EXECUTABLE ${CMAKE_CTEST_COMMAND} -C $<CONFIG> --output-on-failure
            DEPENDENCIES check )
        set ( ENABLE_COVERAGE 1 )
      else()
        message ( SEND_ERROR "Code Coverage is currently only supported for GNU Compiler (GCC)" )
      endif()
    endif ()
  endif (CMAKE_C_COMPILER_ID STREQUAL "Intel" )
endif ( CMAKE_COMPILER_IS_GNUCC OR CMAKE_C_COMPILER_ID MATCHES "Clang" OR CMAKE_C_COMPILER_ID STREQUAL "Intel" )

# Windows
unset ( WINDOWS_LIBS CACHE )
unset ( DSOUND_SUPPORT CACHE )
unset ( WASAPI_SUPPORT CACHE )
unset ( WAVEOUT_SUPPORT CACHE )
unset ( WINMIDI_SUPPORT CACHE )
unset ( MINGW32 CACHE )

if ( CYGWIN )
  find_library(W32API_UUID_LIBRARY uuid
    NO_DEFAULT_PATH
    PATHS /lib/w32api
  )
  if ( NOT W32API_UUID_LIBRARY )
    set ( enable-wasapi off )
  else ()
    message(STATUS "Found W32API uuid library for CYGWIN: " ${W32API_UUID_LIBRARY})
    set ( WINDOWS_LIBS "${WINDOWS_LIBS};${W32API_UUID_LIBRARY}" )
  endif ()
endif ( CYGWIN )

if ( WIN32 OR CYGWIN )
  include ( CheckIncludeFiles )

  # We have to set _WIN32_WINNT to make sure CMake gets correct results when probing for functions.
  # windows-version is supposed to be non-official variable that can be used to tweak the Windows target version.
  # Its value defaults to the Windows Version we are compiling for.
  if ( NOT windows-version )
    if(CMAKE_SYSTEM_VERSION EQUAL 10) # Windows 10
        set ( windows-version "0x0A00" )
    elseif(CMAKE_SYSTEM_VERSION EQUAL 6.3) # Windows 8.1
        set ( windows-version "0x0603" )
    elseif(CMAKE_SYSTEM_VERSION EQUAL 6.2) # Windows 8
        set ( windows-version "0x0602" )
    elseif(CMAKE_SYSTEM_VERSION EQUAL 6.1) # Windows 7
        set ( windows-version "0x0601" )
    elseif(CMAKE_SYSTEM_VERSION EQUAL 6.0) # Windows Vista
        set ( windows-version "0x0600" )
    elseif(CMAKE_SYSTEM_VERSION EQUAL 5.1) # Windows XP
        set ( windows-version "0x0501" )
    else()
        set ( windows-version "0x0400" )
    endif()
  endif ()
  message ( STATUS "Targeting Windows Version ${windows-version}" )
  add_definitions ( -D _WIN32_WINNT=${windows-version} )
  add_definitions ( -D WINVER=${windows-version} )
  list ( APPEND CMAKE_REQUIRED_DEFINITIONS "-DWINVER=${windows-version}" )
  list ( APPEND CMAKE_REQUIRED_DEFINITIONS "-D_WIN32_WINNT=${windows-version}" )

  # Check presence of MS include files
  check_include_file ( windows.h HAVE_WINDOWS_H )
  check_include_file ( io.h HAVE_IO_H )
  check_include_files ( "windows.h;dsound.h" HAVE_DSOUND_H )
  check_include_files ( "windows.h;mmsystem.h" HAVE_MMSYSTEM_H )
  check_include_files ( "mmdeviceapi.h;audioclient.h" HAVE_WASAPI_HEADERS )
  check_include_file ( objbase.h HAVE_OBJBASE_H )

  if ( enable-dsound AND HAVE_DSOUND_H )
    set ( WINDOWS_LIBS "${WINDOWS_LIBS};dsound;ksuser" )
    set ( DSOUND_SUPPORT 1 )
  endif ()

  if ( enable-winmidi AND HAVE_MMSYSTEM_H )
    set ( WINDOWS_LIBS "${WINDOWS_LIBS};winmm" )
    set ( WINMIDI_SUPPORT 1 )
  endif ()

  if ( enable-waveout AND HAVE_MMSYSTEM_H )
    set ( WINDOWS_LIBS "${WINDOWS_LIBS};winmm;ksuser" )
    set ( WAVEOUT_SUPPORT 1 )
  endif ()

  if ( enable-wasapi AND HAVE_WASAPI_HEADERS AND HAVE_OBJBASE_H)
    set ( WINDOWS_LIBS "${WINDOWS_LIBS};ole32" )
    set ( WASAPI_SUPPORT 1 )
  endif ()
endif ( WIN32 OR CYGWIN )

if ( WIN32 )
  if ( enable-network )
    set ( WINDOWS_LIBS "${WINDOWS_LIBS};ws2_32" )
  endif ( enable-network )

  set ( LIBFLUID_CPPFLAGS "-DFLUIDSYNTH_DLL_EXPORTS" )
  set ( FLUID_CPPFLAGS "-DFLUIDSYNTH_NOT_A_DLL" )
  if  ( NOT MSVC )
    # only set debug postfix if not MSVS building
    set ( CMAKE_DEBUG_POSTFIX "_debug" )
  endif ( NOT MSVC )

  # MinGW compiler (a Windows GCC port)
  if ( MINGW )
    set ( MINGW32 1 )
    add_compile_options ( -mms-bitfields )
    # mman-win32
    if ( HAVE_SYS_MMAN_H )
      set ( WINDOWS_LIBS "${WINDOWS_LIBS};mman" )
    endif ()
  endif  ( MINGW )
endif ( WIN32 )

# Check for threads and math
find_package ( Threads REQUIRED )

find_library ( HAS_LIBM NAMES "m" )
if ( HAS_LIBM )
  set ( MATH_LIBRARY "m" )
endif ( HAS_LIBM )

set ( LIBFLUID_LIBS ${MATH_LIBRARY} Threads::Threads )

# IBM OS/2
unset ( DART_SUPPORT CACHE )
unset ( DART_LIBS CACHE )
unset ( DART_INCLUDE_DIRS CACHE )
if ( CMAKE_SYSTEM MATCHES "OS2" )
  set ( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Zbin-files" )
  set ( CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Zbin-files" )
  if ( enable-dart )
    check_include_files ( "os2.h;os2me.h" HAVE_DART_H )
    set ( DART_SUPPORT ${HAVE_DART_H} )
    unset ( DART_INCLUDE_DIRS CACHE )
  endif ( enable-dart )
endif ( CMAKE_SYSTEM MATCHES "OS2" )

# Solaris / SunOS
if ( CMAKE_SYSTEM MATCHES "SunOS" )
  set ( FLUID_LIBS "${FLUID_LIBS};nsl;socket" )
  set ( LIBFLUID_LIBS "${LIBFLUID_LIBS};nsl;socket" )
endif ( CMAKE_SYSTEM MATCHES "SunOS" )

# Apple Mac OSX
unset ( COREAUDIO_SUPPORT CACHE )
unset ( COREAUDIO_LIBS CACHE )
unset ( COREMIDI_SUPPORT CACHE )
unset ( COREMIDI_LIBS CACHE )
unset ( DARWIN CACHE )
unset ( MACOSX_FRAMEWORK CACHE )
if ( CMAKE_SYSTEM MATCHES "Darwin" )
  set ( DARWIN 1 )
  set ( CMAKE_INSTALL_NAME_DIR
        ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR} )
  if ( enable-coreaudio )
    check_include_file ( CoreAudio/AudioHardware.h COREAUDIO_FOUND )
    if ( COREAUDIO_FOUND )
      set ( COREAUDIO_SUPPORT ${COREAUDIO_FOUND} )
      set ( COREAUDIO_LIBS "-Wl,-framework,CoreAudio,-framework,AudioUnit" )
    endif ( COREAUDIO_FOUND )
  endif ( enable-coreaudio )
  if ( enable-coremidi )
    check_include_file ( CoreMIDI/MIDIServices.h COREMIDI_FOUND )
    if ( COREMIDI_FOUND )
      set ( COREMIDI_SUPPORT ${COREMIDI_FOUND} )
      set ( COREMIDI_LIBS "-Wl,-framework,CoreMIDI,-framework,CoreServices" )
    endif ( COREMIDI_FOUND )
  endif ( enable-coremidi )
  if ( enable-framework )
    set ( MACOSX_FRAMEWORK 1 )
  endif ( enable-framework )
endif ( CMAKE_SYSTEM MATCHES "Darwin" )

unset ( NETWORK_SUPPORT )
if ( enable-network )
    set ( NETWORK_SUPPORT 1 )
endif ( enable-network )

unset ( WITH_FLOAT CACHE )
if ( enable-floats )
    set ( WITH_FLOAT 1 )
endif ( enable-floats )

unset ( WITH_PROFILING CACHE )
if ( enable-profiling )
    set ( WITH_PROFILING 1 )
    if ( CMAKE_C_COMPILER_ID STREQUAL "Clang" )
      set ( OPT_FLAGS "-Rpass=loop-vectorize" ) # -Rpass-analysis=loop-vectorize" )
    elseif ( CMAKE_C_COMPILER_ID STREQUAL "Intel" )
      set ( OPT_FLAGS "-qopt-report=3" )
    elseif ( CMAKE_C_COMPILER_ID STREQUAL "GNU" )
      set ( OPT_FLAGS "" )
    endif ( )
    
    set ( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OPT_FLAGS}" )
    
    find_program( CLANG_TIDY
                NAMES "clang-tidy"
                DOC "Path to clang-tidy executable" )

    if ( CLANG_TIDY )
        message ( STATUS "Found clang-tidy at ${CLANG_TIDY}" )
        execute_process ( COMMAND ${CLANG_TIDY} "--version" )
        set ( CMAKE_C_CLANG_TIDY ${CLANG_TIDY} )
    endif ( CLANG_TIDY )

endif ( enable-profiling )

unset ( ENABLE_TRAPONFPE CACHE )
unset ( TRAP_ON_FPE CACHE )
if ( enable-trap-on-fpe AND NOT APPLE AND NOT WIN32 )
    set ( ENABLE_TRAPONFPE 1 )
    set ( TRAP_ON_FPE 1 )
endif ( enable-trap-on-fpe AND NOT APPLE AND NOT WIN32 )

unset ( ENABLE_FPECHECK CACHE )
unset ( FPE_CHECK CACHE )
if ( enable-fpe-check AND NOT APPLE AND NOT WIN32 )
    set ( ENABLE_FPECHECK 1 )
    set ( FPE_CHECK 1 )
endif ( enable-fpe-check AND NOT APPLE AND NOT WIN32 )

if ( NOT CMAKE_BUILD_TYPE )
    set ( CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING
          "Choose the build type, options: Debug Release RelWithDebInfo MinSizeRel" FORCE )
endif ( NOT CMAKE_BUILD_TYPE )

unset ( ENABLE_DEBUG CACHE )
if ( CMAKE_BUILD_TYPE MATCHES "Debug" )
    set ( ENABLE_DEBUG 1 )
    add_definitions(-DDEBUG) # -D_GLIBCXX_DEBUG) # for additional C++ STL container debugging
endif ( CMAKE_BUILD_TYPE MATCHES "Debug" )

# Additional targets to perform clang-format/clang-tidy
# Get all project files
file(GLOB_RECURSE
     ALL_SOURCE_FILES
     LIST_DIRECTORIES false
     ${FluidSynth_SOURCE_DIR}/*.[chi]
     ${FluidSynth_SOURCE_DIR}/*.[chi]pp
     ${FluidSynth_SOURCE_DIR}/*.[chi]xx
     ${FluidSynth_SOURCE_DIR}/*.cc
     ${FluidSynth_SOURCE_DIR}/*.hh
     ${FluidSynth_SOURCE_DIR}/*.ii
     ${FluidSynth_SOURCE_DIR}/*.[CHI]
     )

find_program ( ASTYLE "astyle" )
if ( ASTYLE )
  add_custom_target(
    format
    COMMAND ${ASTYLE}
    -A1
    -xb
    -j
    -k3
    -p
    -f
    -n
    -U
    ${ALL_SOURCE_FILES}
    )
endif(ASTYLE)

# Set the minimum version desired for libraries
set ( ALSA_MINIMUM_VERSION 0.9.1 )
set ( DBUS_MINIMUM_VERSION 1.11.12 )
set ( GLIB2_MINUMUM_VERSION 2.6.5 )
set ( LASH_MINIMUM_VERSION 0.3 )
set ( LIBINSTPATCH_MINIMUM_VERSION 1.1.0 )
set ( LIBSNDFILE_MINIMUM_VERSION 1.0.0 )
set ( PIPEWIRE_MINIMUM_VERSION 0.3 )
set ( PORTAUDIO_MINIMUM_VERSION 2.19 )
set ( PULSEAUDIO_MINIMUM_VERSION 2.0 )

include ( PkgConfigHelpers ) # Needed for Find modules using pkg-config

# Make searching for packages easier on VCPKG
if ( CMAKE_VERSION VERSION_GREATER_EQUAL 3.15 AND VCPKG_TOOLCHAIN )
  set ( CMAKE_FIND_PACKAGE_PREFER_CONFIG ON )
endif ()

# Mandatory libraries: glib and gthread
find_package ( GLib2 ${GLIB2_MINUMUM_VERSION} REQUIRED )
list( APPEND PC_REQUIRES_PRIV "glib-2.0" "gthread-2.0")

if ( GLib2_VERSION AND GLib2_VERSION VERSION_LESS "2.26.0" )
    message ( WARNING "Your version of glib is very old. This may cause problems with fluidsynth's sample cache on Windows. Consider updating to glib 2.26 or newer!" )
endif ( GLib2_VERSION AND GLib2_VERSION VERSION_LESS "2.26.0" )

# Optional features
unset ( LIBSNDFILE_SUPPORT CACHE )
unset ( LIBSNDFILE_HASVORBIS CACHE )
if ( enable-libsndfile )
    find_package ( SndFile ${LIBSNDFILE_MINIMUM_VERSION} )
    set ( LIBSNDFILE_SUPPORT ${SndFile_FOUND} )
    if ( LIBSNDFILE_SUPPORT )
        list( APPEND PC_REQUIRES_PRIV "sndfile")
        if ( SndFile_WITH_EXTERNAL_LIBS )
            set ( LIBSNDFILE_HASVORBIS 1 )
        else (SndFile_WITH_EXTERNAL_LIBS)
            message ( NOTICE "Seems like libsndfile was compiled without OGG/Vorbis support." )
        endif (SndFile_WITH_EXTERNAL_LIBS)
    endif ( LIBSNDFILE_SUPPORT )
endif ( enable-libsndfile )

unset ( PULSE_SUPPORT CACHE )
if ( enable-pulseaudio )
    find_package ( PulseAudio ${PULSEAUDIO_MINIMUM_VERSION} QUIET )
    # Upstream config does not search for pulse-simple
    find_library ( PULSEAUDIO_SIMPLE_LIBRARY NAMES "pulse-simple" )
    if ( PULSEAUDIO_FOUND AND PULSEAUDIO_SIMPLE_LIBRARY )
        set ( PULSE_SUPPORT TRUE )
        set ( PULSEAUDIO_LIBRARIES ${PULSEAUDIO_SIMPLE_LIBRARY} ${PULSEAUDIO_LIBRARY} )
        message ( STATUS "Found PulseAudio: ${PULSEAUDIO_LIBRARIES}" )
        list( APPEND PC_REQUIRES_PRIV "libpulse-simple")
    else ( PULSEAUDIO_FOUND AND PULSEAUDIO_SIMPLE_LIBRARY )
        message( STATUS "Could NOT find PulseAudio (Set PulseAudio_DIR to the directory containing its CMake config)" )
    endif ( PULSEAUDIO_FOUND AND PULSEAUDIO_SIMPLE_LIBRARY )
endif ( enable-pulseaudio )

unset ( ALSA_SUPPORT CACHE )
if ( enable-alsa )
    find_package ( ALSA ${ALSA_MINIMUM_VERSION} )
    set ( ALSA_SUPPORT ${ALSA_FOUND} )
    if ( ALSA_SUPPORT )
        list( APPEND PC_REQUIRES_PRIV "alsa")
    endif ( ALSA_SUPPORT )
endif ( enable-alsa )

unset ( PORTAUDIO_SUPPORT CACHE )
if ( enable-portaudio )
    find_package( PortAudio ${PORTAUDIO_MINIMUM_VERSION} )
    set ( PORTAUDIO_SUPPORT ${PortAudio_FOUND} )
    if ( PORTAUDIO_SUPPORT )
        list( APPEND PC_REQUIRES_PRIV "portaudio-2.0")
    endif ()
endif ( enable-portaudio )

unset ( JACK_SUPPORT CACHE )
if ( enable-jack )
    find_package ( Jack )
    set ( JACK_SUPPORT ${Jack_FOUND} )
    if (JACK_SUPPORT)
        list( APPEND PC_REQUIRES_PRIV "jack")
    endif()
endif ( enable-jack )

unset ( PIPEWIRE_SUPPORT CACHE )
if ( enable-pipewire )
    find_package ( PipeWire ${PIPEWIRE_MINIMUM_VERSION} )
    set ( PIPEWIRE_SUPPORT ${PipeWire_FOUND} )
    if ( PIPEWIRE_SUPPORT )
        list( APPEND PC_REQUIRES_PRIV "libpipewire-0.3")
    endif()
endif ( enable-pipewire )

unset ( LASH_SUPPORT CACHE )
if ( enable-lash )
    find_package ( LASH ${LASH_MINIMUM_VERSION} )
    if ( LASH_FOUND )
        set ( LASH_SUPPORT 1 )
        add_definitions ( -DHAVE_LASH )
        list( APPEND PC_REQUIRES_PRIV "lash-1.0")
    endif ( LASH_FOUND )
endif ( enable-lash )

unset ( SYSTEMD_SUPPORT CACHE )
if ( enable-systemd )
    find_package ( Systemd )
    set ( SYSTEMD_SUPPORT ${Systemd_FOUND} )
    if ( SYSTEMD_SUPPORT )
        list( APPEND PC_REQUIRES_PRIV "libsystemd")
    endif ( SYSTEMD_SUPPORT )
endif ( enable-systemd )

unset ( DBUS_SUPPORT CACHE )
if ( enable-dbus )
    find_package ( DBus1 ${DBUS_MINIMUM_VERSION} QUIET )
    set ( DBUS_SUPPORT ${DBus1_FOUND} )
    if ( DBUS_SUPPORT )
        message ( STATUS "Found DBus1: ${DBus1_LIBRARIES}" )
        list( APPEND PC_REQUIRES_PRIV "dbus-1")
    else ()
        message ( STATUS "Could NOT find DBus1 (Set DBus1_DIR to the directory containing its CMake config)")
    endif ()
endif ( enable-dbus )

unset ( LADSPA_SUPPORT CACHE )
if ( enable-ladspa )
    check_include_file ( ladspa.h HAVE_LADSPA_H )
    if ( HAVE_LADSPA_H )
        if ( NOT TARGET GLib2::gmodule-2 )
            message ( STATUS "LADSPA support was requested but gmodule-2 was not found." )
        else ( NOT TARGET GLib2::gmodule-2 )
            set ( LADSPA_SUPPORT 1 )
            set ( LADSPA 1 )
            list( APPEND PC_REQUIRES_PRIV "gmodule-2.0")
        endif ( NOT TARGET GLib2::gmodule-2 )
    endif ( HAVE_LADSPA_H )
endif ( enable-ladspa )

unset ( LIBINSTPATCH_SUPPORT CACHE )
if ( enable-libinstpatch )
    find_package ( InstPatch ${LIBINSTPATCH_MINIMUM_VERSION} )
    set ( LIBINSTPATCH_SUPPORT ${InstPatch_FOUND} )
    if ( LIBINSTPATCH_SUPPORT )
        list( APPEND PC_REQUIRES_PRIV "libinstpatch-1.0")
    endif (LIBINSTPATCH_SUPPORT )
endif ( enable-libinstpatch )

unset ( SDL2_SUPPORT CACHE )
if ( enable-sdl2 )
    find_package ( SDL2 QUIET )
    if ( SDL2_FOUND )
        if ( NOT DEFINED SDL2_VERSION )
            include ( GetSDL2VersionFromHeaders )
            get_sdl2_version_from_headers("${SDL2_INCLUDE_DIRS}" SDL2_VERSION)
        endif ( NOT DEFINED SDL2_VERSION )
        if ( SDL2_VERSION VERSION_LESS "2.0.4" )
            message ( STATUS "Found SDL2 version ${SDL2_VERSION}, but the minimum required is 2.0.4" )
        else ( SDL2_VERSION VERSION_LESS "2.0.4" )
            message ( STATUS "Found SDL2: ${SDL2_LIBRARIES} (version: ${SDL2_VERSION})" )
            set ( SDL2_SUPPORT TRUE )
            list ( APPEND PC_REQUIRES_PRIV "sdl2")
        endif ( SDL2_VERSION VERSION_LESS "2.0.4" )
    else ( SDL2_FOUND )
        message ( STATUS "Could NOT find SDL2 (Set SDL2_DIR to the directory containing its CMake config)" )
    endif ( SDL2_FOUND )
endif ( enable-sdl2 )

unset ( OBOE_SUPPORT CACHE )
if ( enable-oboe )
  find_package ( oboe )
  if ( oboe_FOUND )
    set(CMAKE_CXX_STANDARD 14)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set ( OBOE_SUPPORT 1 )
    list( APPEND PC_REQUIRES_PRIV "oboe-1.0")
  endif ( oboe_FOUND )
endif ( enable-oboe )

unset ( READLINE_SUPPORT CACHE )
if ( enable-readline )
  find_package ( Readline )
  if ( Readline_FOUND )
    set ( READLINE_SUPPORT 1 )
    if ( PC_READLINE_FOUND )
      list( APPEND PC_REQUIRES_PRIV "readline")
    else ()
      message ( STATUS "No pkg-config file for readline found - trying to make it work anyway." )
      list ( APPEND PC_LIBS_PRIV ${Readline_LIBRARY} )
    endif ()
  endif ( Readline_FOUND )
endif ( enable-readline )

unset ( AUFILE_SUPPORT CACHE )
if ( enable-aufile )
  set ( AUFILE_SUPPORT 1 )
endif ( enable-aufile )

unset ( OSS_SUPPORT CACHE )
if ( enable-oss )
  check_include_file("sys/soundcard.h" HAVE_SYS_SOUNDCARD_H)
  check_include_file("linux/soundcard.h" HAVE_LINUX_SOUNDCARD_H)
  check_include_file("machine/soundcard.h" HAVE_MACHINE_SOUNDCARD_H)
  if ( HAVE_SYS_SOUNDCARD_H OR HAVE_LINUX_SOUNDCARD_H OR HAVE_MACHINE_SOUNDCARD_H )
    set ( OSS_SUPPORT TRUE )
  else ( HAVE_SYS_SOUNDCARD_H OR HAVE_LINUX_SOUNDCARD_H OR HAVE_MACHINE_SOUNDCARD_H )
    set ( OSS_SUPPORT FALSE )
  endif ( HAVE_SYS_SOUNDCARD_H OR HAVE_LINUX_SOUNDCARD_H OR HAVE_MACHINE_SOUNDCARD_H )
endif ( enable-oss )

unset ( MIDISHARE_SUPPORT CACHE )
if ( enable-midishare )
  find_package ( MidiShare QUIET )
  set ( MIDISHARE_SUPPORT ${MidiShare_FOUND} )
endif ( enable-midishare )

unset ( OPENSLES_SUPPORT CACHE )
if ( enable-opensles )
  find_package( OpenSLES )
  set ( OPENSLES_SUPPORT ${OpenSLES_FOUND} )
endif ( enable-opensles )

unset ( ENABLE_MIXER_THREADS CACHE )
if ( enable-threads )
  set ( ENABLE_MIXER_THREADS 1 )
endif ( enable-threads )

unset ( HAVE_OPENMP CACHE )
find_package ( OpenMP COMPONENTS C )
if (enable-openmp AND ENABLE_UBSAN)
    message(WARNING "OpenMP is not supported when UBSan is enabled. Disabling OpenMP.")
elseif (enable-openmp AND OpenMP_C_FOUND )
    message(STATUS "Found OpenMP version: ${OpenMP_C_VERSION} date: ${OpenMP_C_SPEC_DATE}")
    if ( TARGET OpenMP::OpenMP_C AND (( NOT OpenMP_C_SPEC_DATE LESS "201307" ) OR NOT ( OpenMP_C_VERSION VERSION_LESS "4.0" )) )
        set ( HAVE_OPENMP 1 )
        list ( APPEND PC_LIBS_PRIV ${OpenMP_C_LIBRARIES} )
    else()
        message(STATUS "  OpenMP version is not supported. Feature disabled.")
    endif()
endif()

# manipulate some variables to setup a proper test env
set(TEST_SOUNDFONT "${FluidSynth_SOURCE_DIR}/sf2/VintageDreamsWaves-v2.sf2")
set(TEST_SOUNDFONT_UTF8_1 "${FluidSynth_SOURCE_DIR}/sf2/\\xE2\\x96\\xA0VintageDreamsWaves-v2\\xE2\\x96\\xA0.sf2")
set(TEST_SOUNDFONT_UTF8_2 "${FluidSynth_SOURCE_DIR}/sf2/VìntàgèDrèàmsWàvès-v2.sf2")
set(TEST_SOUNDFONT_SF3 "${FluidSynth_SOURCE_DIR}/sf2/VintageDreamsWaves-v2.sf3")
set(TEST_MIDI_UTF8 "${FluidSynth_SOURCE_DIR}/test/èmpty.mid")

# Make sure to link against libm before checking for math functions below
set ( CMAKE_REQUIRED_LIBRARIES "${LIBFLUID_LIBS};${WINDOWS_LIBS}" )

# Check for C99 float math

unset ( HAVE_SINF CACHE )
CHECK_SYMBOL_EXISTS ( sinf "math.h" HAVE_SINF )
if ( HAVE_SINF )
  set ( HAVE_SINF 1 )
endif ( HAVE_SINF )

unset ( HAVE_COSF CACHE )
CHECK_SYMBOL_EXISTS ( cosf "math.h" HAVE_COSF )
if ( HAVE_COSF )
  set ( HAVE_COSF 1 )
endif ( HAVE_COSF )

unset ( HAVE_FABSF CACHE )
CHECK_SYMBOL_EXISTS ( fabsf "math.h" HAVE_FABSF )
if ( HAVE_FABSF )
  set ( HAVE_FABSF 1 )
endif ( HAVE_FABSF )

unset ( HAVE_POWF CACHE )
CHECK_SYMBOL_EXISTS ( powf "math.h" HAVE_POWF )
if ( HAVE_POWF )
  set ( HAVE_POWF 1 )
endif ( HAVE_POWF )

unset ( HAVE_SQRTF CACHE )
CHECK_SYMBOL_EXISTS ( sqrtf "math.h" HAVE_SQRTF )
if ( HAVE_SQRTF )
  set ( HAVE_SQRTF 1 )
endif ( HAVE_SQRTF )

unset ( HAVE_LOGF CACHE )
CHECK_SYMBOL_EXISTS ( logf "math.h" HAVE_LOGF )
if ( HAVE_LOGF )
  set ( HAVE_LOGF 1 )
endif ( HAVE_LOGF )

unset ( HAVE_INETNTOP CACHE )
unset ( IPV6_SUPPORT CACHE )
if ( WIN32 )
  CHECK_SYMBOL_EXISTS ( inet_ntop "ws2tcpip.h" HAVE_INETNTOP )
else ( WIN32 )
  CHECK_SYMBOL_EXISTS ( inet_ntop "arpa/inet.h" HAVE_INETNTOP )
endif ( WIN32 )
if ( enable-ipv6 )
  if ( HAVE_INETNTOP )
    set ( IPV6_SUPPORT 1 )
  endif ( HAVE_INETNTOP )
endif ( enable-ipv6 )

unset ( HAVE_SOCKLEN_T CACHE )
set ( CMAKE_EXTRA_INCLUDE_FILES_SAVE ${CMAKE_EXTRA_INCLUDE_FILES} )
if ( WIN32 )
  set ( CMAKE_EXTRA_INCLUDE_FILES "winsock2.h;ws2tcpip.h" )
else ( WIN32 )
  set ( CMAKE_EXTRA_INCLUDE_FILES sys/socket.h )
endif ( WIN32 )
check_type_size ( socklen_t SOCKLEN_T )
set ( CMAKE_EXTRA_INCLUDE_FILES ${CMAKE_EXTRA_INCLUDE_FILES_SAVE} )
if ( HAVE_SOCKLEN_T )
  set ( HAVE_SOCKLEN_T 1 )
endif ( HAVE_SOCKLEN_T )

# General configuration file
configure_file ( ${FluidSynth_SOURCE_DIR}/src/config.cmake
                 ${FluidSynth_BINARY_DIR}/config.h )

# required to allow ctest to be called from top-level build directory
ENABLE_TESTING()

# Process subdirectories
add_subdirectory ( src )
add_subdirectory ( test )
add_subdirectory ( doc )

# pkg-config support
set ( prefix "${CMAKE_INSTALL_PREFIX}" )
set ( exec_prefix "\${prefix}" )
if ( IS_ABSOLUTE "${CMAKE_INSTALL_LIBDIR}" )
  set ( libdir "${CMAKE_INSTALL_LIBDIR}" )
else ()
  set ( libdir "\${exec_prefix}/${CMAKE_INSTALL_LIBDIR}" )
endif ()
if ( IS_ABSOLUTE "${CMAKE_INSTALL_INCLUDEDIR}" )
  set ( includedir "${CMAKE_INSTALL_INCLUDEDIR}" )
else ()
  set ( includedir "\${prefix}/${CMAKE_INSTALL_INCLUDEDIR}" )
endif ()

generate_pkgconfig_spec(fluidsynth.pc.in ${FluidSynth_BINARY_DIR}/fluidsynth.pc libfluidsynth-OBJ)
install ( FILES ${FluidSynth_BINARY_DIR}/fluidsynth.pc
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig )

# Exported targets for cmake: find_package(FluidSynth)
# when installed, use CMAKE_PREFIX_PATH=fluidsynth-prefix;...
# to use the build directory directly set the FluidSynth_DIR variable instead.

# targets in the build directory
export(EXPORT FluidSynthTargets
   FILE "${FluidSynth_BINARY_DIR}/FluidSynthTargets.cmake"
   NAMESPACE FluidSynth::
)

include(CMakePackageConfigHelpers) # SameMinorVersion requires CMake 3.11
write_basic_package_version_file(
    FluidSynthConfigVersion.cmake
    VERSION ${VERSION}
    COMPATIBILITY SameMinorVersion
)

# In static builds, we need the extra modules so the config can
# find the transitive dependencies.
unset ( EXTRA_STATIC_MODULES )
if ( NOT BUILD_SHARED_LIBS )
  file ( GLOB EXTRA_STATIC_MODULES "${FluidSynth_SOURCE_DIR}/cmake_admin/Find*.cmake" )
  list ( APPEND EXTRA_STATIC_MODULES "${FluidSynth_SOURCE_DIR}/cmake_admin/PkgConfigHelpers.cmake" )
  file ( COPY ${EXTRA_STATIC_MODULES} DESTINATION "${FluidSynth_BINARY_DIR}" )
endif ( NOT BUILD_SHARED_LIBS )

# Here, configure_file() is used instead of the more orthodox macro
# configure_package_config_file() because the latter does not
# support generating a config.cmake file for both the installed
# package and for using the build directory directly.
configure_file(FluidSynthConfig.cmake.in FluidSynthConfig.cmake @ONLY)
install(FILES "${FluidSynth_BINARY_DIR}/FluidSynthConfig.cmake"
              "${FluidSynth_BINARY_DIR}/FluidSynthConfigVersion.cmake"
              ${EXTRA_STATIC_MODULES}
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/fluidsynth")

# Extra targets for Unix build environments
if ( UNIX )
    if ( DEFINED FLUID_DAEMON_ENV_FILE)
        configure_file ( fluidsynth.service.in
        ${FluidSynth_BINARY_DIR}/fluidsynth.service @ONLY )

        configure_file ( fluidsynth.conf.in
        ${FluidSynth_BINARY_DIR}/fluidsynth.conf @ONLY )

    endif ( DEFINED FLUID_DAEMON_ENV_FILE )

    # uninstall custom target
    configure_file ( "${FluidSynth_SOURCE_DIR}/cmake_admin/cmake_uninstall.cmake.in"
      "${FluidSynth_BINARY_DIR}/cmake_uninstall.cmake" IMMEDIATE @ONLY)
    add_custom_target ( uninstall
      "${CMAKE_COMMAND}" -P "${FluidSynth_BINARY_DIR}/cmake_uninstall.cmake")
endif ( UNIX )

include ( report )

# CPack support
set ( CPACK_PACKAGE_DESCRIPTION_SUMMARY "FluidSynth real-time synthesizer" )
set ( CPACK_PACKAGE_VENDOR "fluidsynth.org" )
set ( CPACK_PACKAGE_DESCRIPTION_FILE "${FluidSynth_SOURCE_DIR}/README.md" )
set ( CPACK_RESOURCE_FILE_LICENSE "${FluidSynth_SOURCE_DIR}/LICENSE" )
set ( CPACK_PACKAGE_VERSION_MAJOR ${FLUIDSYNTH_VERSION_MAJOR} )
set ( CPACK_PACKAGE_VERSION_MINOR ${FLUIDSYNTH_VERSION_MINOR} )
set ( CPACK_PACKAGE_VERSION_PATCH ${FLUIDSYNTH_VERSION_MICRO} )
set ( CPACK_PACKAGE_EXECUTABLES "fluidsynth" "FluidSynth CLI" )

# source packages
set ( CPACK_SOURCE_GENERATOR TGZ;TBZ2;ZIP )
set ( CPACK_SOURCE_IGNORE_FILES "/.svn/;/build/;~$;.cproject;.project;/.settings/;${CPACK_SOURCE_IGNORE_FILES}" )
set ( CPACK_SOURCE_PACKAGE_FILE_NAME "${PACKAGE}-${VERSION}" )
set ( CPACK_SOURCE_STRIP_FILES OFF )

# binary packages
include ( InstallRequiredSystemLibraries )
set ( CPACK_GENERATOR STGZ;TGZ;TBZ2;ZIP )
set ( CPACK_PACKAGE_NAME ${PACKAGE} )
set ( CPACK_STRIP_FILES ON )

include ( CPack )
