#[[
// SPDX-License-Identifier: GPL-3.0-only
// MuseScore-CLA-applies
//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2021 MuseScore BVBA and others
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 3 as
//  published by the Free Software Foundation.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <https://www.gnu.org/licenses/>.
//=============================================================================]]

add_custom_target(referenceDocumentation
      ALL
      DEPENDS en/doc.html de/doc.html en/doc.qhp de/doc.qhp
      )

add_custom_command(TARGET referenceDocumentation
      POST_BUILD
      COMMAND Qt5::qcollectiongenerator ${PROJECT_SOURCE_DIR}/rdoc/en/doc.qhcp -o doc_en.qhc
      COMMAND Qt5::qcollectiongenerator ${PROJECT_SOURCE_DIR}/rdoc/de/doc.qhcp -o doc_de.qhc
      )

install(FILES
      ${PROJECT_BINARY_DIR}/rdoc/doc_en.qhc
      ${PROJECT_BINARY_DIR}/rdoc/doc_de.qhc
  DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME}manual)

