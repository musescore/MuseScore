#[[
// SPDX-License-Identifier: GPL-3.0-only
// MuseScore-CLA-applies
//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2021 MuseScore BVBA and others
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 3 as
//  published by the Free Software Foundation.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <https://www.gnu.org/licenses/>.
//=============================================================================]]

include (${PROJECT_SOURCE_DIR}/build/gch.cmake)

if (NOT MSVC)
   set(_all_h_file "${PROJECT_BINARY_DIR}/all.h")
else (NOT MSVC)
   set(_all_h_file "${PROJECT_SOURCE_DIR}/all.h")
endif (NOT MSVC)

add_library (
  qtsingleapp STATIC
  ${_all_h_file}
  ${PCH}
  qtsingleapplication.cpp
  qtlocalpeer.cpp
  )

if (NOT MSVC)
   set_target_properties (
      qtsingleapp
      PROPERTIES
         COMPILE_FLAGS "${PCH_INCLUDE} -g -Wall -Wextra -Winvalid-pch"
      )
else (NOT MSVC)
   set_target_properties (
      qtsingleapp
      PROPERTIES
         COMPILE_FLAGS "${PCH_INCLUDE}"
      )
endif (NOT MSVC)   

xcode_pch(qtsingleapp all)

# Use MSVC pre-compiled headers
vstudio_pch( qtsingleapp )

# MSVC does not depend on mops1 & mops2 for PCH
if (NOT MSVC)
   ADD_DEPENDENCIES(qtsingleapp mops1)
   ADD_DEPENDENCIES(qtsingleapp mops2)
endif (NOT MSVC)   
