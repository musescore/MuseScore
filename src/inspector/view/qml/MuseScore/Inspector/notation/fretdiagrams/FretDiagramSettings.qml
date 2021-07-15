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
import QtQuick 2.15

import MuseScore.UiComponents 1.0
import MuseScore.Inspector 1.0
import "internal"

Item {
    id: root

    property alias model: fretDiagramTabPanel.model

    objectName: "FretDiagramSettings"

    height: content.implicitHeight

    Column {
        id: content

        width: parent.width

        spacing: 12

        FretDiagramTabPanel {
            id: fretDiagramTabPanel

            width: parent.width
        }

        Column {
            height: childrenRect.height
            width: parent.width

            spacing: 12

            FlatButton {
                width: parent.width

                visible: root.model ? root.model.areSettingsAvailable : false

                text: qsTrc("inspector", "Reset")

                onClicked: {
                    fretCanvas.clear()
                }
            }

            FretCanvas {
                id: fretCanvas

                diagram: root.model ? root.model.fretDiagram : null
                isBarreModeOn: root.model ? root.model.isBarreModeOn : false
                isMultipleDotsModeOn: root.model ? root.model.isMultipleDotsModeOn : false
                currentFretDotType: root.model ? root.model.currentFretDotType : false
                visible: root.model ? root.model.areSettingsAvailable : false
                color: ui.theme.fontPrimaryColor

                width: parent.width
            }

        }
    }

    StyledTextLabel {
        anchors.fill: parent

        wrapMode: Text.Wrap
        text: qsTrc("inspector", "You have multiple fretboard diagrams selected. Select a single diagram to edit its settings")
        visible: root.model ? !root.model.areSettingsAvailable : false
    }
}
