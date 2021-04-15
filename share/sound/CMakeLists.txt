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

if (NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/MuseScore_General.sf3)
  install (FILES FluidR3Mono_GM.sf3
      DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME}sound
      RENAME MuseScore_General.sf3
      )
else ()
  install(FILES
      MuseScore_General.sf3
      DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME}sound
      )
endif ()

if (NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/MuseScore_General_License.md)
  install (FILES FluidR3Mono_License.md
      DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME}sound
      RENAME MuseScore_General_License.md)
else ()
  install(FILES
      MuseScore_General_License.md
      DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME}sound
      )
endif ()

