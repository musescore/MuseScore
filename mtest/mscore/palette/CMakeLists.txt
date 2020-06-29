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

set(TARGET tst_palette)

set(MTEST_LINK_MSCOREAPP TRUE)

include(${PROJECT_SOURCE_DIR}/mtest/cmake.inc)

if (MSVC OR MINGW)
      get_target_property(
            MTEST_INSTALL_DESTINATION_DIR
            ${TARGET}
            MTEST_INSTALL_DESTINATION_DIR
            )
      install(FILES
            "${PROJECT_BINARY_DIR}/share/workspaces/Basic.workspace"
            "${PROJECT_BINARY_DIR}/share/workspaces/Advanced.workspace"
            DESTINATION "${MTEST_INSTALL_DESTINATION_DIR}/workspaces"
            PERMISSIONS OWNER_READ GROUP_READ WORLD_READ
            )
endif (MSVC OR MINGW)
