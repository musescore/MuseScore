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
      orchestral.drm
      DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME}templates
      )
install(FILES
      01-General/00-Blank.mscz
      01-General/01-Treble_Clef.mscz
      01-General/02-Bass_Clef.mscz
      01-General/03-Grand_Staff.mscz
      DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME}templates/01-General
      )

install(FILES
      02-Choral/01-SATB.mscz
      02-Choral/02-SATB_+_Organ.mscz
      02-Choral/03-SATB_+_Piano.mscz
      02-Choral/04-SATB_Closed_Score.mscz
      02-Choral/05-SATB_Closed_Score_+_Organ.mscz
      02-Choral/06-SATB_Closed_Score_+_Piano.mscz
      02-Choral/07-Voice_+_Piano.mscz
      02-Choral/08-Barbershop_Quartet.mscz
      02-Choral/09-Liturgical_Unmetrical.mscz
      02-Choral/10-Liturgical_Unmetrical_+_Organ.mscz
      DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME}templates/02-Choral
      )

install(FILES
      03-Chamber_Music/01-String_Quartet.mscz
      03-Chamber_Music/02-Wind_Quartet.mscz
      03-Chamber_Music/03-Wind_Quintet.mscz
      03-Chamber_Music/04-Saxophone_Quartet.mscz
      03-Chamber_Music/05-Brass_Quartet.mscz
      03-Chamber_Music/06-Brass_Quintet.mscz
      DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME}templates/03-Chamber_Music
      )

install(FILES
      04-Solo_Guitar/01-Guitar.mscz
      04-Solo_Guitar/02-Guitar_+_Tablature.mscz
      04-Solo_Guitar/03-Guitar_Tablature.mscz
      DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME}templates/04-Solo_Guitar
      )

install(FILES
      05-Jazz/01-Jazz_Lead_Sheet.mscz
      05-Jazz/02-Big_Band.mscz
      05-Jazz/03-Jazz_Combo.mscz
      DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME}templates/05-Jazz
      )

install(FILES
      06-Popular/01-Rock_Band.mscz
      DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME}templates/06-Popular
      )

install(FILES
      07-Band/01-Concert_Band.mscz
      DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME}templates/07-Band
      )

install(FILES
      08-Orchestral/01-Classical_Orchestra.mscz
      08-Orchestral/02-Concert_Orchestra.mscz
      08-Orchestral/03-String_Orchestra.mscz
      DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME}templates/08-Orchestral
      )

