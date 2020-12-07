#=============================================================================
#  MuseScore
#  Music Composition & Notation
#
#  Copyright (C) 2014 Werner Schweer
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License version 2
#  as published by the Free Software Foundation and appearing in
#  the file LICENSE.GPL
#=============================================================================

set(MODULE qzip)

set(MODULE_SRC
    ${CMAKE_CURRENT_LIST_DIR}/qzip.cpp
    ${CMAKE_CURRENT_LIST_DIR}/qzipreader_p.h
    ${CMAKE_CURRENT_LIST_DIR}/qzipwriter_p.h
    )

set(MODULE_LINK z)

include(${PROJECT_SOURCE_DIR}/build/module.cmake)


#include (${PROJECT_SOURCE_DIR}/build/gch.cmake)

#if (APPLE)
#        file(GLOB_RECURSE INCS "*.h")
#else (APPLE)
#        set(INCS "")
#endif (APPLE)

#if (NOT MSVC)
#   set(_all_h_file "${PROJECT_BINARY_DIR}/all.h")
#else (NOT MSVC)
#   set(_all_h_file "${PROJECT_SOURCE_DIR}/all.h")
#endif (NOT MSVC)

#add_library(qzip STATIC
#   qzip.cpp
#   ${_all_h_file}
#   ${PCH}
#   ${INCS}
#   )

#if (NOT MSVC)
#   set_target_properties (
#      qzip
#      PROPERTIES
#         COMPILE_FLAGS "${PCH_INCLUDE} -g -Wall -Wextra -Winvalid-pch"
#      )
#else (NOT MSVC)
#   set_target_properties (
#      qzip
#      PROPERTIES
#         COMPILE_FLAGS "${PCH_INCLUDE}"
#      )
#      include_directories("${PROJECT_SOURCE_DIR}/dependencies/include/zlib")
#endif (NOT MSVC)

#xcode_pch(qzip all)

## Use MSVC pre-compiled headers
#vstudio_pch( qzip )

## MSVC does not depend on mops1 & mops2 for PCH
#if (NOT MSVC)
#   ADD_DEPENDENCIES(qzip mops1)
#   ADD_DEPENDENCIES(qzip mops2)
#endif (NOT MSVC)

