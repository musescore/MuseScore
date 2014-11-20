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
      DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME}templates
      )
install(FILES
      01-General/01-My_First_Score.mscz
      01-General/02-Treble_Clef.mscz
      01-General/03-Bass_Clef.mscz
      DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME}templates/01-General
      )

install(FILES
      Choral/02-Liturgical_Unmetrical.mscz
      Choral/02-Liturgical_Unmetrical_with_Organ.mscz
      Choral/02-SATB.mscz
      Choral/02-SATB_Closed_Score.mscz
      Choral/02-SATB_Closed_Score_with_Organ.mscz
      Choral/02-SATB_Closed_Score_with_Piano.mscz
      Choral/02-SATB_with_Organ.mscz
      Choral/02-SATB_with_Piano.mscz
      Choral/02-Voice_Piano.mscz
      DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME}templates/Choral
      )

install(FILES
      Solo/03-Guitar.mscz
      Solo/03-Guitar_Standard_Tablature.mscz
      Solo/03-Guitar_Tablature.mscz
      Solo/03-Piano.mscz
      DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME}templates/Solo
      )

install(FILES
      Jazz/04-Jazz_Big_Band.mscz
      Jazz/04-Jazz_Combo.mscz
      Jazz/04-Jazz_Lead_Sheet.mscz
      DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME}templates/Jazz
      )

install(FILES
      Orchestral/05-Classical_Orchestra.mscz
      DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME}templates/Orchestral
      )

install(FILES
      Band/06-Rock_Band.mscz
      DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME}templates/Band
      )

