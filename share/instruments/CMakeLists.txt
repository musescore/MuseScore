#=============================================================================
#  MusE
#  Linux Music Editor
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

install(FILES
      instruments_ca.xml
      instruments_es.xml
      instruments_fr.xml
      instruments_gl.xml
      instruments_it.xml
      instruments_ja.xml
      instruments_nb.xml
      instruments_nl.xml
      instruments_pt.xml
      instruments_pt_BR.xml
      instruments_ru.xml
      instruments_uk.xml
      instruments_zh_CN.xml
      instruments_zh_TW.xml
      instruments.xml
      instruments-band.xml
      DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME}instruments
      )

