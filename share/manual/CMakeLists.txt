#=============================================================================
#  MusE
#  Linux Music Editor
#
#  Copyright (C) 2002-2012 by Werner Schweer and others
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

install(DIRECTORY
      plugins
      DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME}manual
      )

install (DIRECTORY
      ${PROJECT_BINARY_DIR}/share/manual/plugins
      DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME}manual
      )

install(FILES
      DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME}manual
      )

if (IS_DIRECTORY en)
install (DIRECTORY
      en
      DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME}manual
      )
endif (IS_DIRECTORY en)

if (IS_DIRECTORY de)
install (DIRECTORY
      de
      DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME}manual
      )
endif (IS_DIRECTORY de)

