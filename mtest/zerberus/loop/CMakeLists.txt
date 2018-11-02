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

set(TARGET tst_sfzloop)

include(${PROJECT_SOURCE_DIR}/mtest/cmake.inc)

include_directories(
      ${SNDFILE_INCDIR}
      )

target_link_libraries(tst_sfzloop zerberus synthesizer audiofile ${SNDFILE_LIB} testutils)
