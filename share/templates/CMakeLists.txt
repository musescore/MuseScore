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
      drumset_fr.drm
      01-General-Bass_Clef.mscx
      01-General-Treble_Clef.mscx
      02-Choral-Liturgical_Unmetrical.mscx
      02-Choral-Liturgical_Unmetrical_with_Organ.mscx
      02-Choral-SATB.mscx
      02-Choral-SATB_Closed_Score.mscx
      02-Choral-SATB_Closed_Score_with_Organ.mscx
      02-Choral-SATB_Closed_Score_with_Piano.mscx
      02-Choral-SATB_with_Organ.mscx
      02-Choral-SATB_with_Piano.mscx
      02-Choral-Voice_Piano.mscx
      03-Solo-Guitar.mscx
      03-Solo-Guitar_Standard_Tablature.mscx
      03-Solo-Guitar_Tablature.mscx
      03-Solo-Piano.mscx
      04-Jazz-Jazz_Big_Band.mscx
      04-Jazz-Jazz_Combo.mscx
      04-Jazz-Jazz_Lead_Sheet.mscx
      05-Orchestral-Classical_Orchestra.mscx
      06-Band-Rock_Band.mscx
      DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME}templates
      )

