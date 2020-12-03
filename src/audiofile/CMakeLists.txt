#=============================================================================
#  MuseScore
#  Music Composition & Notation
#
#  Copyright (C) 2013 Werner Schweer
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License version 2
#  as published by the Free Software Foundation and appearing in
#  the file LICENSE.GPL
#=============================================================================

set(MODULE audiofile)

set(MODULE_SRC
    ${CMAKE_CURRENT_LIST_DIR}/audiofile.cpp
    ${CMAKE_CURRENT_LIST_DIR}/audiofile.h
    )

include(GetPlatformInfo)

##
## libsndfile
##

set(SNDFILE_LIB )
if (PLATFORM_IS_WINDOWS)

    set(SNDFILE_LIB sndfiledll)

else(PLATFORM_IS_WINDOWS)

    PKGCONFIG1 (sndfile 1.0.25 SNDFILE_INCDIR SNDFILE_LIBDIR SNDFILE_LIB SNDFILE_CPP)
    if (SNDFILE_INCDIR)
       message(STATUS "libsndfile detected ${SNDFILE_INCDIR} ${SNDFILE_LIBDIR} ${SNDFILE_LIB}")
    else (SNDFILE_INCDIR)
       message(FATAL_ERROR "libsndfile not found\n")
    endif (SNDFILE_INCDIR)

endif(PLATFORM_IS_WINDOWS)

set(MODULE_INCLUDE
    ${SNDFILE_INCDIR}
    )

set(MODULE_LINK
    ${SNDFILE_LIB}
    )

include(${PROJECT_SOURCE_DIR}/build/module.cmake)
