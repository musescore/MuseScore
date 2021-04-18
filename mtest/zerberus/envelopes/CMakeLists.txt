#=============================================================================
#  MuseScore
#  Music Composition & Notation
#
#  Copyright (C) 2011 Werner Schweer
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License version 2
#  as published by the Free Software Foundation and appearing in
#  the file LICENSE.GPL
#=============================================================================

set(TARGET tst_sfzenvelopes)

include(${PROJECT_SOURCE_DIR}/mtest/cmake.inc)

include_directories(
      ${SNDFILE_INCDIR}
      )

if (MSVC OR MINGW)
      target_link_libraries(tst_sfzenvelopes audio audiofile sndfiledll testutils)
else (MSVC OR MINGW)
      target_link_libraries(tst_sfzenvelopes audio audiofile ${SNDFILE_LIB} testutils)
endif (MSVC OR MINGW)
