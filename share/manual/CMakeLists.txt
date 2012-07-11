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
      chord.html
      chordrest.html
      cursor.html
      data.html
      element.html
      harmony.html
      measurebase.html
      measure.html
      musescore.html
      note.html
      pageformat.html
      page.html
      part.html
      plugins.html
      qfile.html
      qprocess.html
      rest.html
      score.html
      scoreview.html
      segment.html
      staff.html
      text.html
      timesig.html
      DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME}manual
      )

