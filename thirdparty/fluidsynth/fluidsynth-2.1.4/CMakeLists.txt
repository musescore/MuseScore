# FluidSynth - A Software Synthesizer
#
# Copyright (C) 2003-2011 Peter Hanappe and others.
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

cmake_minimum_required ( VERSION 3.1.0 ) # because of CMAKE_C_STANDARD
project ( FluidSynth C )
set ( CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake_admin )

# FluidSynth package name
set ( PACKAGE "fluidsynth" )

# FluidSynth package version
set ( FLUIDSYNTH_VERSION_MAJOR 2 )
set ( FLUIDSYNTH_VERSION_MINOR 1 )
set ( FLUIDSYNTH_VERSION_MICRO 4 )
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
set ( LIB_VERSION_CURRENT 2 )
set ( LIB_VERSION_AGE 3 )
set ( LIB_VERSION_REVISION 4 )
set ( LIB_VERSION_INFO
      "${LIB_VERSION_CURRENT}.${LIB_VERSION_AGE}.${LIB_VERSION_REVISION}" )

# Options disabled by default
option ( enable-debug "enable debugging (default=no)" off )
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
option ( enable-ipv6  "enable ipv6 support" on )
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
option ( enable-waveout "compile Windows WaveOut support (if it is available)" on )
option ( enable-winmidi "compile Windows MIDI support (if it is available)" on )
option ( enable-sdl2 "compile SDL2 audio support (if it is available)" on )
option ( enable-pkgconfig "use pkg-config to locate fluidsynth's (mostly optional) dependencies" on )
option ( enable-pulseaudio "compile PulseAudio support (if it is available)" on )
option ( enable-readline "compile readline lib line editing (if it is available)" on )
option ( enable-threads "enable multi-threading support (such as parallel voice synthesis)" on )

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

# Initialize the library directory name suffix.
if (NOT MINGW AND NOT MSVC AND NOT CMAKE_SYSTEM_NAME MATCHES "FreeBSD|DragonFly")
if ( CMAKE_SIZEOF_VOID_P EQUAL 8 )
  set ( _init_lib_suffix "64" )
else ( CMAKE_SIZEOF_VOID_P EQUAL 8 )
  set ( _init_lib_suffix "" )
endif ( CMAKE_SIZEOF_VOID_P EQUAL 8 )
else ()
  set ( _init_lib_suffix "" )
endif()
set ( LIB_SUFFIX ${_init_lib_suffix} CACHE STRING
      "library directory name suffix (32/64/nothing)" )
mark_as_advanced ( LIB_SUFFIX )

# the default C standard to use for all targets
set(CMAKE_C_STANDARD 90)

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
if ( POLICY CMP0063 )
  cmake_policy ( SET CMP0063 NEW )
endif ( POLICY CMP0063 )

# Default install directory names
include ( DefaultDirs )

# Basic C library checks
include ( CheckSTDC )
include ( CheckIncludeFile )
include ( CheckFunctionExists )
check_include_file ( string.h HAVE_STRING_H )
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
  if ( NOT APPLE AND NOT OS2 )
    set ( CMAKE_EXE_LINKER_FLAGS
          "${CMAKE_EXE_LINKER_FLAGS} -Wl,--as-needed" )
    set ( CMAKE_SHARED_LINKER_FLAGS
          "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-undefined" )
  endif ( NOT APPLE AND NOT OS2 )

  # define some warning flags
  set ( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -W -Wpointer-arith -Wcast-qual -Wstrict-prototypes -Wno-unused-parameter -Wdeclaration-after-statement -Werror=implicit-function-declaration -Werror=incompatible-pointer-types" )

  # prepend to build type specific flags, to allow users to override
  set ( CMAKE_C_FLAGS_DEBUG "-g ${CMAKE_C_FLAGS_DEBUG}" )

  if ( CMAKE_C_COMPILER_ID STREQUAL "Intel" )
    # icc needs the restrict flag to recognize C99 restrict pointers
    set ( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -restrict" )
  else () # not intel
    # gcc and clang support bad function cast and alignment warnings; add them as well.
    set ( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wbad-function-cast -Wcast-align" )

    if ( enable-ubsan )
      set ( CMAKE_C_FLAGS "-fsanitize=address,undefined ${CMAKE_C_FLAGS}" )
      set ( CMAKE_EXE_LINKER_FLAGS "-fsanitize=address,undefined ${CMAKE_EXE_LINKER_FLAGS}" )
      set ( CMAKE_SHARED_LINKER_FLAGS "-fsanitize=address,undefined ${CMAKE_SHARED_LINKER_FLAGS}" )
      set ( ENABLE_UBSAN 1 )
    endif ( enable-ubsan )
  endif (CMAKE_C_COMPILER_ID STREQUAL "Intel" )
endif ( CMAKE_COMPILER_IS_GNUCC OR CMAKE_C_COMPILER_ID MATCHES "Clang" OR CMAKE_C_COMPILER_ID STREQUAL "Intel" )

# Windows
unset ( WINDOWS_LIBS CACHE )
unset ( DSOUND_SUPPORT CACHE )
unset ( WAVEOUT_SUPPORT CACHE )
unset ( WINMIDI_SUPPORT CACHE )
unset ( MINGW32 CACHE )
if ( WIN32 )
  include ( CheckIncludeFiles )

  # Check presence of MS include files
  check_include_file ( windows.h HAVE_WINDOWS_H )
  check_include_file ( io.h HAVE_IO_H )
  check_include_file ( dsound.h HAVE_DSOUND_H )
  check_include_files ( "windows.h;mmsystem.h" HAVE_MMSYSTEM_H )

  if ( enable-network )
    set ( WINDOWS_LIBS "${WINDOWS_LIBS};ws2_32" )
  endif ( enable-network )

  if ( enable-dsound AND HAVE_DSOUND_H )
    set ( WINDOWS_LIBS "${WINDOWS_LIBS};dsound" )
    set ( DSOUND_SUPPORT 1 )
  endif ()

  if ( enable-winmidi AND HAVE_MMSYSTEM_H )
    set ( WINDOWS_LIBS "${WINDOWS_LIBS};winmm" )
    set ( WINMIDI_SUPPORT 1 )
  endif ()

  if ( enable-waveout AND HAVE_MMSYSTEM_H )
    set ( WINDOWS_LIBS "${WINDOWS_LIBS};winmm" )
    set ( WAVEOUT_SUPPORT 1 )
  endif ()

  set ( LIBFLUID_CPPFLAGS "-DFLUIDSYNTH_DLL_EXPORTS" )
  set ( FLUID_CPPFLAGS "-DFLUIDSYNTH_NOT_A_DLL" )
  if  ( MSVC )
    # statically link in the CRT library to avoid a bunch of runtime DLL dependencies and allow 
    # the CI windows builds to be run under WinXP
    foreach ( flag_var
        CMAKE_C_FLAGS
        CMAKE_C_FLAGS_DEBUG
        CMAKE_C_FLAGS_RELEASE
        CMAKE_C_FLAGS_MINSIZEREL
        CMAKE_C_FLAGS_RELWITHDEBINFO
        CMAKE_CXX_FLAGS
        CMAKE_CXX_FLAGS_DEBUG
        CMAKE_CXX_FLAGS_RELEASE
        CMAKE_CXX_FLAGS_MINSIZEREL
        CMAKE_CXX_FLAGS_RELWITHDEBINFO
        )
      if ( ${flag_var} MATCHES "/MD" )
        string ( REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}" )
      endif ( ${flag_var} MATCHES "/MD" )
    endforeach ( flag_var )

  else ( MSVC )
    # only set debug postfix if not MSVS building
    set ( CMAKE_DEBUG_POSTFIX "_debug" )
  endif ( MSVC )

  # MinGW compiler (a Windows GCC port)
  if ( MINGW )
    set ( MINGW32 1 )
    add_compile_options ( -mms-bitfields )
  endif  ( MINGW )
else ( WIN32 )
# Check PThreads, but not in Windows
  find_package ( Threads REQUIRED )
  set ( LIBFLUID_LIBS "m" ${CMAKE_THREAD_LIBS_INIT} )
endif ( WIN32 )

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
        ${CMAKE_INSTALL_PREFIX}/${LIB_INSTALL_DIR} )
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


unset ( HAVE_INETNTOP CACHE )
unset ( IPV6_SUPPORT CACHE )
CHECK_FUNCTION_EXISTS ( "inet_ntop" HAVE_INETNTOP )
if ( enable-ipv6 )
  if ( HAVE_INETNTOP )
    set ( IPV6_SUPPORT 1 )
  endif ( HAVE_INETNTOP )
endif ( enable-ipv6 )

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
    
    if ( CMAKE_VERSION VERSION_GREATER "3.6.0" )
        find_program( CLANG_TIDY
                    NAMES "clang-tidy"
                    DOC "Path to clang-tidy executable" )

        if ( CLANG_TIDY )
            message ( STATUS "Found clang-tidy at ${CLANG_TIDY}" )
            execute_process ( COMMAND ${CLANG_TIDY} "--version" )
            set ( CMAKE_C_CLANG_TIDY ${CLANG_TIDY} )
        endif ( CLANG_TIDY )
    endif ( CMAKE_VERSION VERSION_GREATER "3.6.0" )

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

if ( enable-debug )
    set ( CMAKE_BUILD_TYPE "Debug" CACHE STRING
          "Choose the build type, options: Debug Release RelWithDebInfo MinSizeRel" FORCE )
endif ( enable-debug )

if ( NOT CMAKE_BUILD_TYPE )
    set ( CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING
          "Choose the build type, options: Debug Release RelWithDebInfo MinSizeRel" FORCE )
endif ( NOT CMAKE_BUILD_TYPE )

unset ( ENABLE_DEBUG CACHE )
if ( CMAKE_BUILD_TYPE MATCHES "Debug" )
    set ( ENABLE_DEBUG 1 )
    add_definitions(-DDEBUG)
endif ( CMAKE_BUILD_TYPE MATCHES "Debug" )

# Additional targets to perform clang-format/clang-tidy
# Get all project files
file(GLOB_RECURSE
     ALL_SOURCE_FILES
     LIST_DIRECTORIES false
     ${CMAKE_SOURCE_DIR}/*.[chi]
     ${CMAKE_SOURCE_DIR}/*.[chi]pp
     ${CMAKE_SOURCE_DIR}/*.[chi]xx
     ${CMAKE_SOURCE_DIR}/*.cc
     ${CMAKE_SOURCE_DIR}/*.hh
     ${CMAKE_SOURCE_DIR}/*.ii
     ${CMAKE_SOURCE_DIR}/*.[CHI]
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

if(NOT enable-pkgconfig)

    FIND_LIBRARY( GLIB_LIB NAMES glib glib-2.0 PATH GLIB_LIBRARY_DIR )
    FIND_LIBRARY( GTHREAD_LIB NAMES gthread gthread-2.0 PATH GTHREAD_LIBRARY_DIR )
    FIND_PATH( GLIBH_DIR glib.h PATH GLIB_INCLUDE_DIR )
    FIND_PATH( GLIBCONF_DIR glibconfig.h PATH GLIBCONF_INCLUDE_DIR )

    IF( GLIB_LIB MATCHES "GLIB_LIB-NOTFOUND" OR
            GTHREAD_LIB MATCHES "GTHREAD_LIB-NOTFOUND" OR
            GLIBH_DIR MATCHES "GLIBH_DIR-NOTFOUND" OR
            GLIBCONF_DIR MATCHES "GLIBCONF_DIR-NOTFOUND")
            message( WARNING "Not sure if I found GLIB, continuing anyway.")
    ENDIF()

    SET( GLIB_INCLUDE_DIRS ${GLIBH_DIR} ${GLIBCONF_DIR} )
    SET( GLIB_LIBRARIES ${GLIB_LIB} ${GTHREAD_LIB} )

    message( STATUS "GLIB_INCLUDE_DIRS: " ${GLIB_INCLUDE_DIRS} )
    message( STATUS "GLIB_LIBRARIES: " ${GLIB_LIBRARIES} )

else(NOT enable-pkgconfig)

    find_package ( PkgConfig REQUIRED )

    # Mandatory libraries: glib and gthread
    pkg_check_modules ( GLIB REQUIRED glib-2.0>=2.6.5 gthread-2.0>=2.6.5 )

    if ( GLIB_glib-2.0_VERSION AND GLIB_glib-2.0_VERSION VERSION_LESS "2.26.0" )
        message ( WARNING "Your version of glib is very old. This may cause problems with fluidsynth's sample cache on Windows. Consider updating to glib 2.26 or newer!" )
    endif ( GLIB_glib-2.0_VERSION AND GLIB_glib-2.0_VERSION VERSION_LESS "2.26.0" )

    include ( UnsetPkgConfig )

    # Optional features
    unset ( LIBSNDFILE_SUPPORT CACHE )
    unset ( LIBSNDFILE_HASVORBIS CACHE )
    if ( enable-libsndfile )
    pkg_check_modules ( LIBSNDFILE sndfile>=1.0.0 )
    set ( LIBSNDFILE_SUPPORT ${LIBSNDFILE_FOUND} )
    if ( LIBSNDFILE_SUPPORT )
        pkg_check_modules ( LIBSNDFILE_VORBIS sndfile>=1.0.18 )
        set ( LIBSNDFILE_HASVORBIS ${LIBSNDFILE_VORBIS_FOUND} )
    endif ( LIBSNDFILE_SUPPORT )
    else ( enable-libsndfile )
    unset_pkg_config ( LIBSNDFILE )
    unset_pkg_config ( LIBSNDFILE_VORBIS )
    endif ( enable-libsndfile )

    unset ( PULSE_SUPPORT CACHE )
    if ( enable-pulseaudio )
    pkg_check_modules ( PULSE libpulse-simple>=0.9.8 )
    set ( PULSE_SUPPORT ${PULSE_FOUND} )
    else ( enable-pulseaudio )
    unset_pkg_config ( PULSE )
    endif ( enable-pulseaudio )

    unset ( ALSA_SUPPORT CACHE )
    if ( enable-alsa )
    pkg_check_modules ( ALSA alsa>=0.9.1 )
    set ( ALSA_SUPPORT ${ALSA_FOUND} )
    else ( enable-alsa )
    unset_pkg_config ( ALSA )
    endif ( enable-alsa )

    unset ( PORTAUDIO_SUPPORT CACHE )
    if ( enable-portaudio )
    pkg_check_modules ( PORTAUDIO portaudio-2.0>=19 )
    set ( PORTAUDIO_SUPPORT ${PORTAUDIO_FOUND} )
    else ( enable-portaudio )
    unset_pkg_config ( PORTAUDIO )
    endif ( enable-portaudio )

    unset ( JACK_SUPPORT CACHE )
    if ( enable-jack )
    pkg_check_modules ( JACK jack )
    set ( JACK_SUPPORT ${JACK_FOUND} )
    else ( enable-jack )
    unset_pkg_config ( JACK )
    endif ( enable-jack )

    unset ( LASH_SUPPORT CACHE )
    if ( enable-lash )
    pkg_check_modules ( LASH lash-1.0>=0.3 )
    if ( LASH_FOUND )
        set ( LASH_SUPPORT 1 )
        add_definitions ( -DHAVE_LASH )
    endif ( LASH_FOUND )
    else ( enable-lash )
    unset_pkg_config ( LASH )
    remove_definitions( -DHAVE_LASH )
    endif ( enable-lash )

    unset ( SYSTEMD_SUPPORT CACHE )
    if ( enable-systemd )
        pkg_check_modules ( SYSTEMD libsystemd )
        set ( SYSTEMD_SUPPORT ${SYSTEMD_FOUND} )
    else ( enable-systemd )
        unset_pkg_config ( SYSTEMD )
    endif ( enable-systemd )

    unset ( DBUS_SUPPORT CACHE )
    if ( enable-dbus )
    pkg_check_modules ( DBUS dbus-1>=1.0.0 )
    set ( DBUS_SUPPORT ${DBUS_FOUND} )
    else ( enable-dbus )
    unset_pkg_config ( DBUS )
    endif ( enable-dbus )

    unset ( LADSPA_SUPPORT CACHE )
    if ( enable-ladspa )
    check_include_file ( ladspa.h LADSPA_SUPPORT )
    if ( LADSPA_SUPPORT )
        pkg_check_modules ( GMODULE REQUIRED gmodule-2.0>=2.6.5 )
        set ( LADSPA 1 )
    endif ( LADSPA_SUPPORT )
    endif ( enable-ladspa )

    unset ( LIBINSTPATCH_SUPPORT CACHE )
    if ( enable-libinstpatch )
        pkg_check_modules ( LIBINSTPATCH libinstpatch-1.0>=1.1.0 )
        set ( LIBINSTPATCH_SUPPORT ${LIBINSTPATCH_FOUND} )
    endif ( enable-libinstpatch )

    unset ( SDL2_SUPPORT CACHE )
    if ( enable-sdl2 )
        pkg_check_modules ( SDL2 sdl2 )
        set ( SDL2_SUPPORT ${SDL2_FOUND} )
    else ( enable-sdl2 )
        unset_pkg_config ( SDL2 )
    endif ( enable-sdl2 )

    unset ( OBOE_SUPPORT CACHE )
    unset ( OBOE_LIBS CACHE )
    if ( enable-oboe )
    # enable C++ as it's needed for oboe
    enable_language ( CXX )
    pkg_check_modules ( OBOE oboe-1.0 )
    if ( OBOE_FOUND )
        set ( OBOE_SUPPORT 1 )
        set ( OBOE_LIBS ${OBOE_LIBRARIES} )
    endif ( OBOE_FOUND )
    endif ( enable-oboe )

    unset ( WITH_READLINE CACHE )
    unset ( READLINE_LIBS CACHE )
    if ( enable-readline )
    pkg_check_modules ( READLINE readline )
    if ( NOT READLINE_FOUND )
        find_package ( READLINE )
        set ( READLINE_FOUND ${HAVE_READLINE} )
    endif ( NOT READLINE_FOUND )
    if ( READLINE_FOUND )
        set ( WITH_READLINE 1 )
        set ( READLINE_LIBS ${READLINE_LIBRARIES} )
    endif ( READLINE_FOUND )
    endif ( enable-readline )

endif(NOT enable-pkgconfig)

unset ( AUFILE_SUPPORT CACHE )
if ( enable-aufile )
  set ( AUFILE_SUPPORT 1 )
endif ( enable-aufile )

unset ( OSS_SUPPORT CACHE )
if ( enable-oss )
  find_package ( OSS QUIET )
  set ( OSS_SUPPORT ${OSS_FOUND} )
endif ( enable-oss )

unset ( MIDISHARE_SUPPORT CACHE )
if ( enable-midishare )
  find_package ( MidiShare QUIET )
  set ( MIDISHARE_SUPPORT ${MidiShare_FOUND} )
  if ( MidiShare_FOUND )
    set ( MidiShare_LIBS ${MidiShare_LIBRARIES} )
  else ( MidiShare_FOUND )
    unset ( MidiShare_LIBS CACHE )
  endif ( MidiShare_FOUND )
else ( enable-midishare )
  unset ( MidiShare_LIBS CACHE )
endif ( enable-midishare )

unset ( OPENSLES_SUPPORT CACHE )
unset ( OpenSLES_LIBS CACHE )
if ( enable-opensles )
  check_include_file ( SLES/OpenSLES.h OPENSLES_SUPPORT )
  if ( OPENSLES_SUPPORT )
    find_library ( OpenSLES_LIBS OpenSLES )
    if ( NOT OpenSLES_LIBS )
      unset ( OPENSLES_SUPPORT )
    endif ( NOT OpenSLES_LIBS )
  endif ( OPENSLES_SUPPORT )
endif ( enable-opensles )

unset ( ENABLE_MIXER_THREADS CACHE )
if ( enable-threads )
  set ( ENABLE_MIXER_THREADS 1 )
endif ( enable-threads )

unset ( HAVE_OPENMP CACHE )
find_package ( OpenMP QUIET )
if ( OpenMP_FOUND OR OpenMP_C_FOUND )
    message(STATUS "Found OpenMP ${OpenMP_C_SPEC_DATE}")
    # require at least OMP 4.0
    if ( ( NOT OpenMP_C_SPEC_DATE LESS "201307" ) OR NOT ( OpenMP_C_VERSION VERSION_LESS "4.0" ) )
        set ( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}" )
        set ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}" )

        # currently no need to link against openMP runtime lib(s). If need be, uncomment below.
        #  set ( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}" )
        #  set ( LIBFLUID_LIBS "${OpenMP_C_LIBRARIES};${LIBFLUID_LIBS}" )
        set ( HAVE_OPENMP 1 )
    endif()
endif()

# manipulate some variables to setup a proper test env
set(TEST_SOUNDFONT "${CMAKE_SOURCE_DIR}/sf2/VintageDreamsWaves-v2.sf2")
set(TEST_SOUNDFONT_SF3 "${CMAKE_SOURCE_DIR}/sf2/VintageDreamsWaves-v2.sf3")

# Check for C99 float math

unset ( HAVE_SINF CACHE )
CHECK_FUNCTION_EXISTS ( "sinf" HAVE_SINF )
if ( HAVE_SINF )
  set ( HAVE_SINF 1 )
endif ( HAVE_SINF )

unset ( HAVE_COSF CACHE )
CHECK_FUNCTION_EXISTS ( "cosf" HAVE_COSF )
if ( HAVE_COSF )
  set ( HAVE_COSF 1 )
endif ( HAVE_COSF )

unset ( HAVE_FABSF CACHE )
CHECK_FUNCTION_EXISTS ( "fabsf" HAVE_FABSF )
if ( HAVE_FABSF )
  set ( HAVE_FABSF 1 )
endif ( HAVE_FABSF )

unset ( HAVE_POWF CACHE )
CHECK_FUNCTION_EXISTS ( "powf" HAVE_POWF )
if ( HAVE_POWF )
  set ( HAVE_POWF 1 )
endif ( HAVE_POWF )

unset ( HAVE_SQRTF CACHE )
CHECK_FUNCTION_EXISTS ( "sqrtf" HAVE_SQRTF )
if ( HAVE_SQRTF )
  set ( HAVE_SQRTF 1 )
endif ( HAVE_SQRTF )

unset ( HAVE_LOGF CACHE )
CHECK_FUNCTION_EXISTS ( "logf" HAVE_LOGF )
if ( HAVE_LOGF )
  set ( HAVE_LOGF 1 )
endif ( HAVE_LOGF )

# General configuration file
configure_file ( ${CMAKE_SOURCE_DIR}/src/config.cmake
                 ${CMAKE_BINARY_DIR}/config.h )

# Setup linker directories NOW, as the command will apply only to targets created after it has been called.
link_directories (
    ${GLIB_LIBRARY_DIRS}
    ${LASH_LIBRARY_DIRS}
    ${JACK_LIBRARY_DIRS}
    ${ALSA_LIBRARY_DIRS}
    ${PULSE_LIBRARY_DIRS}
    ${PORTAUDIO_LIBRARY_DIRS}
    ${LIBSNDFILE_LIBRARY_DIRS}
    ${DBUS_LIBRARY_DIRS}
    ${SDL2_LIBRARY_DIRS}
    ${OBOE_LIBRARY_DIRS}
    ${LIBINSTPATCH_LIBRARY_DIRS}
)

# Process subdirectories
add_subdirectory ( src )
add_subdirectory ( test )
add_subdirectory ( doc )

# pkg-config support
set ( prefix "${CMAKE_INSTALL_PREFIX}" )
set ( exec_prefix "\${prefix}" )
if ( IS_ABSOLUTE "${LIB_INSTALL_DIR}" )
  set ( libdir "${LIB_INSTALL_DIR}" )
else ()
  set ( libdir "\${exec_prefix}/${LIB_INSTALL_DIR}" )
endif ()
if ( IS_ABSOLUTE "${INCLUDE_INSTALL_DIR}" )
  set ( includedir "${INCLUDE_INSTALL_DIR}" )
else ()
  set ( includedir "\${prefix}/${INCLUDE_INSTALL_DIR}" )
endif ()
configure_file ( fluidsynth.pc.in
	${CMAKE_BINARY_DIR}/fluidsynth.pc IMMEDIATE @ONLY )
install ( FILES ${CMAKE_BINARY_DIR}/fluidsynth.pc
	DESTINATION ${LIB_INSTALL_DIR}/pkgconfig )

# Extra targets for Unix build environments
if ( UNIX )
    # RPM spec
    configure_file ( fluidsynth.spec.in
        ${CMAKE_BINARY_DIR}/fluidsynth.spec IMMEDIATE @ONLY )

    if ( DEFINED FLUID_DAEMON_ENV_FILE)
        configure_file ( fluidsynth.service.in
        ${CMAKE_BINARY_DIR}/fluidsynth.service @ONLY )

        configure_file ( fluidsynth.conf.in
        ${CMAKE_BINARY_DIR}/fluidsynth.conf @ONLY )

    endif ( DEFINED FLUID_DAEMON_ENV_FILE )

    # uninstall custom target
    configure_file ( "${CMAKE_SOURCE_DIR}/cmake_admin/cmake_uninstall.cmake.in"
      "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake" IMMEDIATE @ONLY)
    add_custom_target ( uninstall
      "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")

    # tarball custom target
    add_custom_target ( tarball
        COMMAND mkdir -p ${PACKAGE}-${VERSION}
        COMMAND cp -r bindings ${PACKAGE}-${VERSION}
        COMMAND cp -r cmake_admin ${PACKAGE}-${VERSION}
        COMMAND cp -r doc ${PACKAGE}-${VERSION}
        COMMAND cp -r include ${PACKAGE}-${VERSION}
        COMMAND cp -r src ${PACKAGE}-${VERSION}
        COMMAND cp AUTHORS ChangeLog CMakeLists.txt LICENSE ${PACKAGE}.* INSTALL NEWS README* THANKS TODO ${PACKAGE}-${VERSION}
#       COMMAND tar -cj --exclude .svn --exclude Makefile.am -f ${PACKAGE}-${VERSION}.tar.bz2 ${PACKAGE}-${VERSION}
#       COMMAND tar -cz --exclude .svn --exclude Makefile.am -f ${PACKAGE}-${VERSION}.tar.gz ${PACKAGE}-${VERSION}
#       COMMAND zip -qr ${PACKAGE}-${VERSION}.zip ${PACKAGE}-${VERSION} -x '*.svn*' -x '*Makefile.am'
        COMMAND tar -cj --exclude .svn -f ${PACKAGE}-${VERSION}.tar.bz2 ${PACKAGE}-${VERSION}
        COMMAND tar -cz --exclude .svn -f ${PACKAGE}-${VERSION}.tar.gz ${PACKAGE}-${VERSION}
        COMMAND zip -qr ${PACKAGE}-${VERSION}.zip ${PACKAGE}-${VERSION} -x '*.svn*'
        COMMAND rm -rf ${PACKAGE}-${VERSION}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )

endif ( UNIX )

include ( report )

# CPack support
set ( CPACK_PACKAGE_DESCRIPTION_SUMMARY "FluidSynth real-time synthesizer" )
set ( CPACK_PACKAGE_VENDOR "fluidsynth.org" )
set ( CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_SOURCE_DIR}/README.md" )
set ( CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE" )
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
