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

set(TARGET tst_biab)

include(${PROJECT_SOURCE_DIR}/mtest/cmake.inc)

get_target_property(
      MTEST_INSTALL_DESTINATION_DIR
      ${TARGET}
      MTEST_INSTALL_DESTINATION_DIR
      )
install(DIRECTORY
      ${PROJECT_SOURCE_DIR}/share/styles
      DESTINATION "${MTEST_INSTALL_DESTINATION_DIR}"
      FILES_MATCHING PATTERN "*.xml"
      )
