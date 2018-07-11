#=============================================================================
#  MusE
#  Linux Music Editor
#  $Id:$
#
#  Copyright (C) 2002-2006 by Werner Schweer and others
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License version 2.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#=============================================================================

include (${PROJECT_SOURCE_DIR}/build/gch.cmake)

include_directories(
      ${PROJECT_SOURCE_DIR}
      ${PROJECT_BINARY_DIR}
      )

if (OCR)
      set (OCR_SRC ocr.cpp)
endif (OCR)

if (NOT MSVC)
   set(_all_h_file "${PROJECT_BINARY_DIR}/all.h")
else (NOT MSVC)
   set(_all_h_file "${PROJECT_SOURCE_DIR}/all.h")
endif (NOT MSVC)

add_library (
      omr STATIC
      ${_all_h_file}
      ${PCH}
      omrview.cpp pdf.cpp omrpage.cpp
      skew.cpp utils.cpp
      omr.cpp pattern.cpp importpdf.cpp
      ${OCR_SRC}
      )

if (NOT MSVC)
   set_target_properties (
      omr
      PROPERTIES
         COMPILE_FLAGS "${PCH_INCLUDE} -g -Wall -Wextra -Winvalid-pch -Wno-unused-private-field"
      )
else (NOT MSVC)
   set_target_properties (
      omr
      PROPERTIES
         COMPILE_FLAGS "${PCH_INCLUDE}"    # TODO: Add disabling of unused private field warning?
      )
endif (NOT MSVC)   

xcode_pch(omr all)

# Use MSVC pre-compiled headers
vstudio_pch( omr )

# MSVC does not depend on mops1 & mops2 for PCH
if (NOT MSVC)
   ADD_DEPENDENCIES(omr mops1)
   ADD_DEPENDENCIES(omr mops2)
endif (NOT MSVC)   

