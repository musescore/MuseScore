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
      01-General/00-Blank.mscz
      01-General/01-Treble_Clef.mscz
      01-General/02-Bass_Clef.mscz
      DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME}templates/01-General
      )

install(FILES
      02-Choral/01-SATB.mscz
      02-Choral/02-SATB_+_Organ.mscz
      02-Choral/03-SATB_+_Piano.mscz
      02-Choral/04-SATB_Closed_Score.mscz
      02-Choral/05-SATB_Closed_Score_+_Organ.mscz
      02-Choral/06-SATB_Closed_Score_+_Piano.mscz
      02-Choral/07-Voice_Piano.mscz
      02-Choral/08-Liturgical_Unmetrical.mscz
      02-Choral/09-Liturgical_Unmetrical_+_Organ.mscz
      DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME}templates/02-Choral
      )

install(FILES
      03-Solo/01-Guitar.mscz
      03-Solo/02-Guitar_+_Tablature.mscz
      03-Solo/03-Guitar_Tablature.mscz
      03-Solo/04-Piano.mscz
      DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME}templates/03-Solo
      )

install(FILES
      04-Jazz/01-Jazz_Lead_Sheet.mscz
      04-Jazz/02-Big_Band.mscz
      04-Jazz/03-Jazz_Combo.mscz
      DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME}templates/04-Jazz
      )

install(FILES
      05-Orchestral/01-Classical_Orchestra.mscz
      05-Orchestral/02-String_Quartet.mscz
      DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME}templates/05-Orchestral
      )

install(FILES
      06-Band/01-Rock_Band.mscz
      DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME}templates/06-Band
      )

