#=============================================================================
#  Mscore
#  Linux Music Score Editor
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

QT5_WRAP_UI (aeolusUi aeolus_gui.ui)

add_library (aeolus STATIC
      ${aeolusUi}
      aeolus.cpp audio.cpp model.cpp addsynth.cpp scales.cpp
      asection.cpp division.cpp rankwave.cpp
      rngen.cpp exp2ap.cpp
      aeolusgui.cpp sparm.cpp
      ${INCS}
      )

if (BUILD_PCH)
    target_use_pch(aeolus)
endif (BUILD_PCH)

install(
   DIRECTORY stops
   DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME}sound/aeolus
   PATTERN .svn EXCLUDE
   )
