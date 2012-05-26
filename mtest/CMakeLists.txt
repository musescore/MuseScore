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

set(CMAKE_VERBOSE_MAKEFILE ON)
enable_testing()

include_directories(
      ${PROJECT_BINARY_DIR}
      ${PROJECT_SOURCE_DIR}
      )

add_library(
      testutils STATIC
      testutils.cpp
      mcursor.cpp
      ${PROJECT_SOURCE_DIR}/mscore/importmidi.cpp
      ${PROJECT_SOURCE_DIR}/omr/importpdf.cpp
      ${PROJECT_SOURCE_DIR}/mscore/midifile.cpp
      ${PROJECT_SOURCE_DIR}/mscore/exportmidi.cpp
      omr
      )

add_executable(
      mtest
      mtest.cpp
      )
target_link_libraries(
      mtest
      ${QT_LIBRARIES}
      )

set_target_properties (
      testutils
      PROPERTIES
      COMPILE_FLAGS "-include all.h -I${QT_HEADERS_DIR} -D TESTROOT=\\\"${PROJECT_SOURCE_DIR}\\\" -g -Wall -Wextra"
      )

add_custom_target(report
      COMMAND ant -f ${PROJECT_SOURCE_DIR}/mtest/build.xml -Droot.dir=${PROJECT_BINARY_DIR}/mtest report
      WORKING_DIRECTORY "${PROJECT_BINARY_DIR}/mtest"
      )
add_custom_target(reporthtml
      COMMAND ant -f ${PROJECT_SOURCE_DIR}/mtest/build.xml -Droot.dir=${PROJECT_BINARY_DIR}/mtest reporthtml
      WORKING_DIRECTORY "${PROJECT_BINARY_DIR}/mtest"
      )

subdirs(libmscore omr)


