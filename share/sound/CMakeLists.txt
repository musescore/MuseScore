#=============================================================================
#  MuseScore
#  Music Score Editor
#  $Id:$
#
#  Copyright (C) 2002-2008 by Werner Schweer and others
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

if (NOT EXISTS MS_General.sf3)
  install (FILES FluidR3Mono_GM.sf3
      DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME}sound
      RENAME MS_General.sf3
      )
else ()
  install(FILES
      MS_General.sf3
      DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME}sound
      )
endif ()

if (NOT EXISTS MS_General-License.md)
  install (FILES FluidR3Mono_License.md
      DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME}sound
      RENAME MS_General-License.md)
else ()
  install(FILES
      MS_General-License.md
      DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME}sound
      )
endif ()

