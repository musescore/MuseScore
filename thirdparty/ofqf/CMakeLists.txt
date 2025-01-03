#=============================================================================
#  MusE
#  Linux Music Editor
#
#  Copyright (C) 2010 by Werner Schweer and others
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

add_library(ofqf STATIC
    ${INCS}
    qoscclient.cpp qoscserver.cpp qosctypes.cpp
    )

if (BUILD_PCH)
    target_use_pch(ofqf)
endif (BUILD_PCH)

if (APPLE)
  set_target_properties( ofqf PROPERTIES
    COMPILE_FLAGS "-Wno-shorten-64-to-32"
  )
endif (APPLE)

