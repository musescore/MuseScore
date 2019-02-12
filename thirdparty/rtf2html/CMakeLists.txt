#=============================================================================
#  Mscore
#  Linux Music Score Editor
#
#  Copyright (C) 2009 by Werner Schweer and others
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

add_library (
  rtf2html STATIC
  ${_all_h_file}
  ${INCS}
  fmt_opts.cpp
  rtf2html.cpp
  rtf_keyword.cpp
  rtf_table.cpp
  )

if (NOT MSVC)
   set_target_properties( rtf2html
      PROPERTIES
         COMPILE_FLAGS "${PCH_INCLUDE} -g -Wall -Wextra -Winvalid-pch"
      )
else (NOT MSVC)
   set_target_properties ( rtf2html
      PROPERTIES
         COMPILE_FLAGS "${PCH_INCLUDE} /wd4267"
      )
endif (NOT MSVC)   

xcode_pch(rtf2html all)

# Use MSVC pre-compiled headers
vstudio_pch( rtf2html )

# MSVC does not depend on mops1 & mops2 for PCH
if (NOT MSVC)
   ADD_DEPENDENCIES(rtf2html mops1)
   ADD_DEPENDENCIES(rtf2html mops2)
endif (NOT MSVC)   

