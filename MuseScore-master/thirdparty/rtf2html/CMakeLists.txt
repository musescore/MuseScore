#=============================================================================
#  Mscore
#  Linux Music Score Editor
#  $Id:$
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

add_library (
  rtf2html STATIC
  ${PROJECT_BINARY_DIR}/all.h
  ${INCS}
  fmt_opts.cpp
  rtf2html.cpp
  rtf_keyword.cpp
  rtf_table.cpp
  )

set_target_properties( rtf2html
   PROPERTIES
      COMPILE_FLAGS "${PCH_INCLUDE} -g -Wall -Wextra -Winvalid-pch"
   )

xcode_pch(rtf2html all)

ADD_DEPENDENCIES(rtf2html mops1)
ADD_DEPENDENCIES(rtf2html mops2)

