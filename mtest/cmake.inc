#=============================================================================
#  MuseScore
#  Music Composition & Notation
#  $Id:$
#
#  Copyright (C) 2011 Werner Schweer
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License version 2
#  as published by the Free Software Foundation and appearing in
#  the file LICENSE.GPL
#=============================================================================

QT4_ADD_RESOURCES(qrc_files ${PROJECT_SOURCE_DIR}/mtest/mtest.qrc)

add_executable(
      ${TARGET}
      ${ui_headers}
      ${qrc_files}
      ${mocs}
      ${TARGET}.cpp
      )

target_link_libraries(
      ${TARGET}
      ${QT_LIBRARIES}
      ${QT_QTTEST_LIBRARY}
      testutils
      libmscore
      msynth
      zarchive
      z
      dl
      pthread
      omr
      fitz
      jbig2dec
      jpeg
      openjpeg
      fontconfig
      freetype
      )

set_target_properties (
      ${TARGET}
      PROPERTIES
      AUTOMOC true
      COMPILE_FLAGS "-include all.h -D TESTROOT=\\\"${PROJECT_SOURCE_DIR}\\\" -g -Wall -Wextra"
      LINK_FLAGS    "-g"
      )
add_test(${TARGET} ${CMAKE_CURRENT_BINARY_DIR}/${TARGET}  -xunitxml -o result.xml)
