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

include (${PROJECT_SOURCE_DIR}/build/gch.cmake)

if (APPLE)
        file(GLOB_RECURSE INCS "*.h")
else (APPLE)
        set(INCS "")
endif (APPLE)

if (NOT MSVC)
   set(_all_h_file "${PROJECT_BINARY_DIR}/all.h")
else (NOT MSVC)
   set(_all_h_file "${PROJECT_SOURCE_DIR}/all.h")
endif (NOT MSVC)

add_library (audiofile STATIC
      ${_all_h_file}
      ${PCH}
      audiofile.cpp
      )

if (NOT MSVC)
   set_target_properties (
      audiofile
      PROPERTIES
         COMPILE_FLAGS "${PCH_INCLUDE} -g -Wall -Wextra -Winvalid-pch"
      )
else (NOT MSVC)
   set_target_properties (
      audiofile
      PROPERTIES
         COMPILE_FLAGS "${PCH_INCLUDE}"
      )
endif (NOT MSVC)   

xcode_pch(audiofile all)

# Use MSVC pre-compiled headers
vstudio_pch( audiofile )

# MSVC does not depend on mops1 & mops2 for PCH
if (NOT MSVC)
   ADD_DEPENDENCIES(audiofile mops1)
   ADD_DEPENDENCIES(audiofile mops2)
endif (NOT MSVC)


