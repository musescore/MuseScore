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

# Scripts
install(FILES
      TC1.1_CreateSimpleScore.js
      TC1.2_CreateSimpleScoreWithRandomInstruments.js
      TC2_CreateSimpleScoreByTemplate.js
      TC3_UsingNoteInputToolbar.js
      TC4_UsingPalettes.js
      TC5_UsingInstruments.js
      TC6_UsingInspector_Note.js
      TC7_UsingExport.js
      TC8_EngravingText.js
      "TC9_BigScore(perfomance).js"
      DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME}autobotscripts
      )

# Steps
install(FILES
      steps/NewScore.js
      steps/NoteInput.js
      steps/Palette.js
      steps/Score.js
      steps/Instruments.js
      steps/Inspector.js
      steps/Navigation.js
      steps/Home.js
      DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME}autobotscripts/steps
      )

# Date
install(FILES
      data/Big_Score.mscz
      DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME}autobotscripts/data
      )
