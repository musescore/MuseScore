# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-Studio-CLA-applies
#
# MuseScore Studio
# Music Composition & Notation
#
# Copyright (C) 2021 MuseScore Limited
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 3 as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

# adding or renaming templates here should also better be done in convert.json, 
# but definitely need to get reflected in ../instruments/instrumentsxml.h,
# like by running ../instruments/update_instruments_xml.py

install(FILES
      Marching_Bass_Drums.drm
      Marching_Cymbals.drm
      Marching_Snare_Drums.drm
      Marching_Tenors.drm
      drumset_fr.drm
      orchestral.drm
      My_First_Score.mscx
      categories.json
      DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME}templates
      )
install(DIRECTORY
      01-General/01-Treble_Clef
      01-General/02-Bass_Clef
      01-General/03-Grand_Staff
      DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME}templates/01-General
      )

install(DIRECTORY
      02-Choral/01-SATB
      02-Choral/02-SATB_+_Organ
      02-Choral/03-SATB_+_Piano
      02-Choral/04-SATB_Closed_Score
      02-Choral/05-SATB_Closed_Score_+_Organ
      02-Choral/06-SATB_Closed_Score_+_Piano
      02-Choral/07-Voice_+_Piano
      02-Choral/08-Barbershop_Quartet_\(Men\)
      02-Choral/09-Barbershop_Quartet_\(Women\)
      02-Choral/10-Liturgical_Unmetrical
      02-Choral/11-Liturgical_Unmetrical_+_Organ
      DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME}templates/02-Choral
      )

install(DIRECTORY
      03-Chamber_Music/01-String_Quartet
      03-Chamber_Music/02-Wind_Quartet
      03-Chamber_Music/03-Wind_Quintet
      03-Chamber_Music/04-Saxophone_Quartet
      03-Chamber_Music/05-Brass_Quartet
      03-Chamber_Music/06-Brass_Quintet
      DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME}templates/03-Chamber_Music
      )

install(DIRECTORY
      04-Solo/01-Guitar
      04-Solo/02-Guitar_+_Tablature
      04-Solo/03-Guitar_Tablature
      04-Solo/04-Piano
      DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME}templates/04-Solo
      )

install(DIRECTORY
      05-Jazz/01-Jazz_Lead_Sheet
      05-Jazz/02-Big_Band
      05-Jazz/03-Jazz_Combo
      DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME}templates/05-Jazz
      )

install(DIRECTORY
      06-Popular/01-Rock_Band
      06-Popular/02-Bluegrass_Band
      DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME}templates/06-Popular
      )

install(DIRECTORY
      07-Band_and_Percussion/01-Concert_Band
      07-Band_and_Percussion/02-Small_Concert_Band
      07-Band_and_Percussion/03-Brass_Band
      07-Band_and_Percussion/04-Marching_Band
      07-Band_and_Percussion/05-Small_Marching_Band
      07-Band_and_Percussion/06-Battery_Percussion
      07-Band_and_Percussion/07-Large_Pit_Percussion
      07-Band_and_Percussion/08-Small_Pit_Percussion
      07-Band_and_Percussion/09-European_Concert_Band
      DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME}templates/07-Band_and_Percussion
      )

install(DIRECTORY
      08-Orchestral/01-Classical_Orchestra
      08-Orchestral/02-Symphony_Orchestra
      08-Orchestral/03-String_Orchestra
      DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME}templates/08-Orchestral
      )
