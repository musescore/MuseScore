#=============================================================================
#  MusE
#  Linux Music Editor
#  $Id:$
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

install(FILES
      plugins/manual.css
      plugins/chord.html
      plugins/chordrest.html
      plugins/cursor.html
      plugins/data.html
      plugins/element.html
      plugins/harmony.html
      plugins/measurebase.html
      plugins/measure.html
      plugins/musescore.html
      plugins/note.html
      plugins/pageformat.html
      plugins/page.html
      plugins/part.html
      plugins/plugins.html
      plugins/qfile.html
      plugins/qprocess.html
      plugins/rest.html
      plugins/score.html
      plugins/scoreview.html
      plugins/segment.html
      plugins/staff.html
      plugins/text.html
      plugins/timesig.html
      DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME}manual
      )
install (DIRECTORY
      en de
      DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME}manual
      )

