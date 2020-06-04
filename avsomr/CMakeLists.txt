#=============================================================================
#  MuseScore
#  Music Composition & Notation
#
#  Copyright (C) 2020 MuseScore BVBA and others
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
      ${PROJECT_SOURCE_DIR}/framework/global
      )

if (NOT MSVC)
   set(_all_h_file "${PROJECT_BINARY_DIR}/all.h")
else (NOT MSVC)
   set(_all_h_file "${PROJECT_SOURCE_DIR}/all.h")
endif (NOT MSVC)

add_library (
      avsomr STATIC
      ${_all_h_file}
      ${PCH}
      avsimport.cpp
      avslog.h
      ret.cpp
      ret.h
      avsomr.cpp
      avsomr.h
      avsomrreader.cpp
      avsomrreader.h
      msmrfile.cpp
      msmrfile.h
      msmrwriter.cpp
      msmrwriter.h
      avsomrdrawer.cpp
      avsomrdrawer.h
      iavsomrrecognizer.h
      avsomrnetrecognizer.cpp
      avsomrnetrecognizer.h
      avsomrlocal.cpp
      avsomrlocal.h
      avsomrlocalrecognizer.cpp
      avsomrlocalrecognizer.h
      avsomrlocalinstaller.cpp
      avsomrlocalinstaller.h
      avsomrsetup.cpp
      avsomrsetup.h
      ui/recognitionproccessdialog.cpp
      ui/recognitionproccessdialog.h
      ui/taskbarprogress.cpp
      ui/taskbarprogress.h
      ui/infopopup.cpp
      ui/infopopup.h
      ui/setupavsomrview.cpp
      ui/setupavsomrview.h
      )

if (NOT MSVC)
   set_target_properties (
      avsomr
      PROPERTIES
         COMPILE_FLAGS "${PCH_INCLUDE} -g -Wall -Wextra -Winvalid-pch"
      )
else (NOT MSVC)
   set_target_properties (
      avsomr
      PROPERTIES
         COMPILE_FLAGS "${PCH_INCLUDE}"
      )
endif (NOT MSVC)   

xcode_pch(avsomr all)

# Use MSVC pre-compiled headers
vstudio_pch( avsomr )

# MSVC does not depend on mops1 & mops2 for PCH
if (NOT MSVC)
   ADD_DEPENDENCIES(avsomr mops1)
   ADD_DEPENDENCIES(avsomr mops2)
endif (NOT MSVC)   

