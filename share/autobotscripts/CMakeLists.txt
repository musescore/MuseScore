# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-CLA-applies
#
# MuseScore
# Music Composition & Notation
#
# Copyright (C) 2021 MuseScore BVBA and others
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
      DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME}autobotscripts/steps
      )

