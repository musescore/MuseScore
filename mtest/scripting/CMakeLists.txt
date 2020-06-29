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

set(TARGET tst_scripting)

set(MTEST_LINK_MSCOREAPP TRUE)

include(${PROJECT_SOURCE_DIR}/mtest/cmake.inc)

if (MSVC OR MINGW)
      get_target_property(
            MTEST_INSTALL_DESTINATION_DIR
            ${TARGET}
            MTEST_INSTALL_DESTINATION_DIR
            )
      install(DIRECTORY
            ${QT_INSTALL_QML}
            DESTINATION "${MTEST_INSTALL_DESTINATION_DIR}"
            REGEX ".*d\\.dll" EXCLUDE
            REGEX ".*QtMultimedia.*" EXCLUDE
            REGEX ".*QtSensors.*" EXCLUDE
            REGEX ".*QtTest.*" EXCLUDE
            REGEX ".*QtWebkit.*" EXCLUDE
            )
endif (MSVC OR MINGW)
