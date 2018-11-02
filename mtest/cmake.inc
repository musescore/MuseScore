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

add_executable(
      ${TARGET}
      ${ui_headers}
      ${mocs}
      ${TARGET}.cpp
      )

target_link_libraries(
      ${TARGET}
      ${QT_QTTEST_LIBRARY}
      testutils
      testResources
      libmscore
      synthesizer
      midi
      qzip
      z
      )

if (OMR)
      target_link_libraries(${TARGET} omr poppler)
      if (OCR)
            target_link_libraries(${TARGET} tesseract_api)
      endif (OCR)
endif (OMR)

target_link_libraries(
      ${TARGET}
      ${QT_LIBRARIES}
      mscore_freetype
      z
      )

if (NOT MINGW AND NOT APPLE)
   target_link_libraries(${TARGET}
      dl
      pthread)
endif (NOT MINGW AND NOT APPLE)

if (APPLE)
target_link_libraries(${TARGET} ${OsxFrameworks})
target_link_libraries(${TARGET}
      dl
      pthread
      )
set_target_properties (
      ${TARGET}
      PROPERTIES
      AUTOMOC true
      COMPILE_FLAGS "-include all.h -D QT_GUI_LIB -D TESTROOT=\\\"${PROJECT_SOURCE_DIR}\\\" -g -Wall -Wextra"
      LINK_FLAGS    "-g -stdlib=libc++"
      )
else(APPLE)
set_target_properties (
      ${TARGET}
      PROPERTIES
      AUTOMOC true
      COMPILE_FLAGS "-include all.h -D QT_GUI_LIB -D TESTROOT=\\\"${PROJECT_SOURCE_DIR}\\\" -g -Wall -Wextra"
      LINK_FLAGS    "-g"
      )
endif(APPLE)

if (APPLE AND (CMAKE_VERSION VERSION_LESS "3.5.0"))
set_target_properties (
      ${TARGET}
      PROPERTIES
      COMPILE_FLAGS "-include all.h -D QT_GUI_LIB -D TESTROOT=\\\\\"${PROJECT_SOURCE_DIR}\\\\\" -g -Wall -Wextra"
      )
endif (APPLE AND (CMAKE_VERSION VERSION_LESS "3.5.0"))

add_test(${TARGET} ${CMAKE_CURRENT_BINARY_DIR}/${TARGET}  -xunitxml -o result.xml)
