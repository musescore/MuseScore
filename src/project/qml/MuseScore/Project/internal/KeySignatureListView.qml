/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
import QtQuick 2.9

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Project 1.0

GridView {
    id: root
    
    property var currentSignature: null
    property var mode: null
    signal signatureSelected(var signature)

    height: contentHeight

    clip: true
    
    cellWidth: 82
    cellHeight: 90
    
    interactive: height < contentHeight
    
    delegate: ListItemBlank {
        height: root.cellHeight
        width: root.cellWidth

        itemBorderColor: ui.theme.strokeColor
        itemBorderWidth: ui.theme.borderWidth

        radius: 3
        isSelected: modelData.titleMajor === currentSignature.titleMajor

        KeySignature {
            icon: modelData.icon
            text: root.mode === "major" ? modelData.titleMajor : modelData.titleMinor
        }
        
        onClicked: {
            root.signatureSelected(modelData)
        }
    }
}
