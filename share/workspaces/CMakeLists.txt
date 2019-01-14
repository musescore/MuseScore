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

function (build_workspace WS_NAME _TARGET)
      set(_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${WS_NAME}.workspace")
      set(_FILES workspace.xml META-INF/container.xml)
      add_custom_command(OUTPUT ${_OUTPUT}
            COMMAND ${CMAKE_COMMAND} -E tar cf "${_OUTPUT}" --format=zip -- ${_FILES}
            DEPENDS
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${WS_NAME}"
            COMMENT "Building ${WS_NAME}.workspace"
            )
      set("${_TARGET}" "${_OUTPUT}" PARENT_SCOPE)
endfunction(build_workspace)

build_workspace(Basic BASIC_TARGET)
build_workspace(Advanced ADVANCED_TARGET)

add_custom_target(workspaces DEPENDS
      "${BASIC_TARGET}"
      "${ADVANCED_TARGET}"
      )

install(FILES
      "${BASIC_TARGET}"
      "${ADVANCED_TARGET}"
      DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME}workspaces
      PERMISSIONS OWNER_READ GROUP_READ WORLD_READ
      )
