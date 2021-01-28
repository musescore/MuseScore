#=============================================================================
#  MusE
#  Linux Music Editor
#
#  Copyright (C) 2002-2010 by Werner Schweer and others
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

install(FILES
      Leland.mss
      MuseJazz.mss
      legacy-style-defaults-v3.mss
      legacy-style-defaults-v2.mss
      legacy-style-defaults-v1.mss
      chords_std.xml
      chords_jazz.xml
      chords.xml
      stdchords.xml
      jazzchords.xml
      cchords_muse.xml
      cchords_nrb.xml
      cchords_rb.xml
      cchords_sym.xml
      DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME}styles
      )

