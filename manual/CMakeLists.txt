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

include_directories(
      ${PROJECT_BINARY_DIR}
      ${PROJECT_SOURCE_DIR}
      )
add_executable(
      genManual
      genManual.cpp
      )
target_link_libraries(
      genManual
      ${QT_LIBRARIES}
      )

if(APPLE)
set_target_properties (
      genManual
      PROPERTIES
      COMPILE_FLAGS "-include all.h -I${QT_HEADERS_DIR} -D TESTROOT=\\\\\"${PROJECT_SOURCE_DIR}\\\\\" -g -Wall -Wextra"
      )
else(APPLE)
set_target_properties (
      genManual
      PROPERTIES
      COMPILE_FLAGS "-include all.h -I${QT_HEADERS_DIR} -D TESTROOT=\\\"${PROJECT_SOURCE_DIR}\\\" -g -Wall -Wextra"
      )
endif(APPLE)