#=============================================================================
#  MuseScore
#  Music Composition & Notation
#
#  Copyright (C) 2018-2019 Werner Schweer and others
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

function(build_workspace # compress a workspace XML file at build time
  WS_NAME # name of the workspace directory to be zipped
  )
  fn__copy_during_build(
    "${CMAKE_CURRENT_SOURCE_DIR}/${WS_NAME}.xml"
    "${WS_NAME}/${WS_NAME}.xml"
    )
  fn__build_container(
    "${WS_NAME}.workspace"
    "${WS_NAME}"
    "${WS_NAME}.xml"
    )
  list(APPEND GENERATED_WORKSPACES "${WS_NAME}.workspace")
  set(GENERATED_WORKSPACES "${GENERATED_WORKSPACES}" PARENT_SCOPE)
endfunction(build_workspace)

set(GENERATED_WORKSPACES "") # create empty list

build_workspace(Basic)
build_workspace(Advanced)

# custom commands do not run unless something depends on their output
add_custom_target(workspaces DEPENDS ${GENERATED_WORKSPACES})

foreach(FILE IN LISTS GENERATED_WORKSPACES)
  install(
    FILES "${CMAKE_CURRENT_BINARY_DIR}/${FILE}"
    DESTINATION "${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME}workspaces"
    PERMISSIONS OWNER_READ GROUP_READ WORLD_READ
    )
endforeach(FILE)
