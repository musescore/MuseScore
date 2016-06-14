# - Try to find pulseaudio-2.6
# Once done this will define
#
#  PULSEAUDIO_FOUND - system has pulseaudio
#  PULSEAUDIO_INCLUDE_DIRS - the pulseaudio include directory
#  PULSEAUDIO_LIBRARIES - Link these to use pulseaudio
#  PULSEAUDIO_DEFINITIONS - Compiler switches required for using pulseaudio
#
#  Copyright (c) 2008 Andreas Schneider <mail@cynapses.org>
#  Modified for other libraries by Lasse Kärkkäinen <tronic>
#
#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#  For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#
if (NOT MINGW AND NOT APPLE)
  if (PULSEAUDIO_LIBRARIES AND PULSEAUDIO_INCLUDE_DIRS)
    # In cache already
    set(PULSEAUDIO_FOUND TRUE)
  else (PULSEAUDIO_LIBRARIES AND PULSEAUDIO_INCLUDE_DIRS)
    # Use 'pkg-config' to get the directories and then use these values
    # in the FIND_PATH() and FIND_LIBRARY() calls
    if (CMAKE_TOOLCHAIN_FILE)
      include (UsePkgConfig)
      pkgconfig (libpulse PULSEAUDIO_INCLUDEDIRS PULSEAUDIO_LIBDIR PULSEAUDIO_LDFLAGS PULSEAUDIO_CFLAGS)
      set (PULSEAUDIO_LIBRARY "${PULSEAUDIO_LIBDIR}/libpulse.so" )
      set (PULSEAUDIO_FOUND TRUE)
    else (CMAKE_TOOLCHAIN_FILE)
      if (${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} EQUAL 4)
        include(UsePkgConfig)
        pkgconfig(libpulse _PULSEAUDIO_INCLUDEDIR _PULSEAUDIO_LIBDIR _PULSEAUDIO_LDFLAGS _PULSEAUDIO_CFLAGS)
      else (${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} EQUAL 4)
        find_package(PkgConfig)
        if (PKG_CONFIG_FOUND)
          pkg_check_modules(_PULSEAUDIO libpulse)
        endif (PKG_CONFIG_FOUND)
      endif (${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} EQUAL 4)
      find_path(PULSEAUDIO_INCLUDE_DIR
      NAMES
        pulse/pulseaudio.h
      PATHS
        ${_PULSEAUDIO_INCLUDEDIR}
        /usr/include
        /usr/local/include
        /opt/local/include
        /sw/include
      )

      find_library(PULSEAUDIO_LIBRARY
      NAMES
        pulse
      PATHS
        ${_PULSEAUDIO_LIBDIR}
        /usr/lib
        /usr/local/lib
        /opt/local/lib
        /sw/lib
      )

      find_library(PULSEAUDIO_SIMPLE_LIBRARY
      NAMES
        pulse-simple
      PATHS
        ${_PULSEAUDIO_LIBDIR}
        /usr/lib
        /usr/local/lib
        /opt/local/lib
        /sw/lib
      )

      if (PULSEAUDIO_LIBRARY)
        set(PULSEAUDIO_FOUND TRUE)
      endif (PULSEAUDIO_LIBRARY)

      set(PULSEAUDIO_INCLUDE_DIRS
      ${PULSEAUDIO_INCLUDE_DIR}
      )
    endif (CMAKE_TOOLCHAIN_FILE)

    if (PULSEAUDIO_FOUND)
      set(PULSEAUDIO_LIBRARIES
        ${PULSEAUDIO_LIBRARIES}
        ${PULSEAUDIO_SIMPLE_LIBRARY}
        ${PULSEAUDIO_LIBRARY}
      )
    endif (PULSEAUDIO_FOUND)

    if (PULSEAUDIO_INCLUDE_DIRS AND PULSEAUDIO_LIBRARIES)
       set(PULSEAUDIO_FOUND TRUE)
    endif (PULSEAUDIO_INCLUDE_DIRS AND PULSEAUDIO_LIBRARIES)

    if (PULSEAUDIO_FOUND)
      if (NOT PULSEAUDIO_FIND_QUIETLY)
        message(STATUS "Found pulseaudio. LIB ${PULSEAUDIO_LIBRARY}, INCLUDE ${PULSEAUDIO_INCLUDEDIRS}, LIBDIR ${PULSEAUDIO_LIBDIR}, LDFLAGS ${PULSEAUDIO_LDFLAGS}, CFLAGS ${PULSEAUDIO_CFLAGS}")
      endif (NOT PULSEAUDIO_FIND_QUIETLY)
    else (PULSEAUDIO_FOUND)
      if (PULSEAUDIO_FIND_REQUIRED)
        message(FATAL_ERROR "Could not find PulseAudio")
      endif (PULSEAUDIO_FIND_REQUIRED)
    endif (PULSEAUDIO_FOUND)

    # Show the PULSEAUDIO_INCLUDE_DIRS and PULSEAUDIO_LIBRARIES variables only in the advanced view
    mark_as_advanced(PULSEAUDIO_INCLUDE_DIRS PULSEAUDIO_LIBRARIES)

  endif (PULSEAUDIO_LIBRARIES AND PULSEAUDIO_INCLUDE_DIRS)
endif (NOT MINGW AND NOT APPLE)

