#=============================================================================
#  MusE
#  Linux Music Editor
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

# adding or renaming templates here should also better be done in convert.json, 
# but definitely need to get reflected in ../instruments/instrumentsxml.h,
# like by running ../instruments/generateTs.py

install(FILES
      drumset_fr.drm
      orchestral.drm
      My_First_Score.mscx
      Create_New_Score.mscz
      DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME}templates
      )
install(FILES
      01-General/00-Blank.mscx
      01-General/01-Treble_Clef.mscx
      01-General/02-Bass_Clef.mscx
      01-General/03-Grand_Staff.mscx
      DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME}templates/01-General
      )

install(FILES
      02-Choral/01-SATB.mscx
      02-Choral/02-SATB_+_Organ.mscx
      02-Choral/03-SATB_+_Piano.mscx
      02-Choral/04-SATB_Closed_Score.mscx
      02-Choral/05-SATB_Closed_Score_+_Organ.mscx
      02-Choral/06-SATB_Closed_Score_+_Piano.mscx
      02-Choral/07-Voice_+_Piano.mscx
      02-Choral/08-Barbershop_Quartet_\(Men\).mscx
      02-Choral/09-Barbershop_Quartet_\(Women\).mscx
      02-Choral/10-Liturgical_Unmetrical.mscx
      02-Choral/11-Liturgical_Unmetrical_+_Organ.mscx
      DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME}templates/02-Choral
      )

install(FILES
      03-Chamber_Music/01-String_Quartet.mscx
      03-Chamber_Music/02-Wind_Quartet.mscx
      03-Chamber_Music/03-Wind_Quintet.mscx
      03-Chamber_Music/04-Saxophone_Quartet.mscx
      03-Chamber_Music/05-Brass_Quartet.mscx
      03-Chamber_Music/06-Brass_Quintet.mscx
      DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME}templates/03-Chamber_Music
      )

install(FILES
      04-Solo/01-Guitar.mscx
      04-Solo/02-Guitar_+_Tablature.mscx
      04-Solo/03-Guitar_Tablature.mscx
      04-Solo/04-Piano.mscx
      DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME}templates/04-Solo
      )

install(FILES
      05-Jazz/01-Jazz_Lead_Sheet.mscx
      05-Jazz/02-Big_Band.mscx
      05-Jazz/03-Jazz_Combo.mscx
      DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME}templates/05-Jazz
      )

install(FILES
      06-Popular/01-Rock_Band.mscx
      06-Popular/02-Bluegrass_Band.mscx
      DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME}templates/06-Popular
      )

install(FILES
      07-Band_and_Percussion/01-Concert_Band.mscx
      07-Band_and_Percussion/02-Small_Concert_Band.mscx
      07-Band_and_Percussion/03-Brass_Band.mscx
      07-Band_and_Percussion/04-Marching_Band.mscx
      07-Band_and_Percussion/05-Small_Marching_Band.mscx
      07-Band_and_Percussion/06-Battery_Percussion.mscx
      07-Band_and_Percussion/07-Large_Pit_Percussion.mscx
      07-Band_and_Percussion/08-Small_Pit_Percussion.mscx
      07-Band_and_Percussion/09-European_Concert_Band.mscx
      DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME}templates/07-Band_and_Percussion
      )

install(FILES
      08-Orchestral/01-Classical_Orchestra.mscx
      08-Orchestral/02-Symphony_Orchestra.mscx
      08-Orchestral/03-String_Orchestra.mscx
      DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME}templates/08-Orchestral
      )

