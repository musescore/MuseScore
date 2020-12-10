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

set(MODULE_INCLUDE
    ${SNDFILE_INCDIR}
    )

set(MODULE_LINK
    ${SNDFILE_LIB}
    )

include(${PROJECT_SOURCE_DIR}/build/module.cmake)
