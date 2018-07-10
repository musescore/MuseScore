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

include (${PROJECT_SOURCE_DIR}/build/CopyFilesMacros.cmake)

include_directories(
      ${PROJECT_BINARY_DIR}
      ${PROJECT_SOURCE_DIR}
      )

if (APPLE)
        file(GLOB_RECURSE INCS "*.h")
        set(GETOPT_REPLACE "")
else (APPLE)
        set(INCS "")
        if (MSVC)
            set(GETOPT_REPLACE "getopt/getopt.c")
            include_directories("getopt")
        else (MSVC)
            set(GETOPT_REPLACE "")
        endif (MSVC)
endif (APPLE)

add_executable(
      genManual
      ${INCS}
      genManual.cpp
      ${GETOPT_REPLACE}
      )
target_link_libraries(
      genManual
      ${QT_LIBRARIES}
      )

if(APPLE)
set_target_properties (
      genManual
      PROPERTIES
      COMPILE_FLAGS "-include all.h -D TESTROOT=\\\\\"${PROJECT_SOURCE_DIR}\\\\\" -g -Wall -Wextra"
      )
else(APPLE)
   if (NOT MSVC)
      set_target_properties (
         genManual
         PROPERTIES
         COMPILE_FLAGS "-include all.h -D TESTROOT=\\\"${PROJECT_SOURCE_DIR}\\\" -g -Wall -Wextra"
         )
   else (NOT MSVC)
      set_target_properties (
         genManual
         PROPERTIES
         COMPILE_FLAGS "${PCH_INCLUDE} -D TESTROOT=\\\"${PROJECT_SOURCE_DIR}\\\" -DSTATIC_GETOPT"
         )

   endif (NOT MSVC)
endif(APPLE)

if(NOT MSVC)
	ADD_DEPENDENCIES(genManual mops1)
endif(NOT MSVC)

add_custom_target(pluginDocumentation
      ALL
      DEPENDS genManual
      )

add_custom_command(TARGET pluginDocumentation
      POST_BUILD
      COMMAND ${PROJECT_BINARY_DIR}/manual/${CMAKE_CFG_INTDIR}/genManual ${PROJECT_SOURCE_DIR} ${PROJECT_BINARY_DIR}/share/manual
      )
