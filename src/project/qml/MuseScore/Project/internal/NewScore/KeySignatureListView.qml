/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

import Muse.Ui
import Muse.UiComponents
import MuseScore.Project

GridView {
    id: root
    
    property var currentSignature: null
    property string mode: ""
    signal signatureSelected(var signature)

    property int rows: Math.max(0, Math.floor(root.height / root.cellHeight))
    property int columns: Math.max(0, Math.floor(root.width / root.cellWidth))

    property NavigationPanel navigationPanel: NavigationPanel {
        enabled: root.enabled && root.visible
        direction: NavigationPanel.Both
    }

    height: contentHeight

    Layout.margins: -prv.spacing / 2
    anchors.margins: -prv.spacing / 2

    clip: true
    
    cellWidth: 82 + prv.spacing
    cellHeight: 90 + prv.spacing
    
    interactive: height < contentHeight
    
    QtObject {
        id: prv

        readonly property int spacing: 4
    }

    delegate: Item {
        id: delegateItem

        required property var modelData
        required property int index

        height: root.cellHeight
        width: root.cellWidth

        ListItemBlank {
            id: item
            anchors.centerIn: parent
            height: root.cellHeight - prv.spacing
            width: root.cellWidth - prv.spacing

            radius: 3
            isSelected: delegateItem.modelData.titleMajor === root.currentSignature.titleMajor

            navigation.name: keySignature.text
            navigation.panel: root.navigationPanel
            navigation.row: root.columns === 0 ? 0 : Math.floor(delegateItem.index / root.columns)
            navigation.column: delegateItem.index - (navigation.row * root.columns)
            navigation.accessible.name: {
                if (isSelected) {
                    return keySignature.text;
                } else {
                    return keySignature.text + " " + qsTrc("project/newscore", "Not selected");
                }
            }

            KeySignature {
                id: keySignature
                icon: delegateItem.modelData.icon
                text: root.mode === "major" ? delegateItem.modelData.titleMajor : delegateItem.modelData.titleMinor
            }

            onClicked: {
                root.signatureSelected(delegateItem.modelData)
            }
        }
    }
}
