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

if (APPLE)
    file(GLOB_RECURSE INCS "*.h")
else (APPLE)
    set(INCS "")
endif (APPLE)

add_library (rtf2html STATIC
    ${INCS}
    fmt_opts.cpp
    rtf2html.cpp
    rtf_keyword.cpp
    rtf_table.cpp
    )

if (BUILD_PCH)
    target_use_pch(rtf2html)
endif (BUILD_PCH)

if (MSVC)
  set_target_properties( rtf2html PROPERTIES
    COMPILE_FLAGS "/wd4267"
  )
elseif (APPLE)
  set_target_properties( rtf2html PROPERTIES
    COMPILE_FLAGS "-Wno-shorten-64-to-32 -Wno-deprecated-declarations"
  )
endif ()
