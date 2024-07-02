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
import QtQuick 2.15
import QtQuick.Controls 2.15

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import MuseScore.Inspector 1.0

import "../../../common"

FocusableItem {
    id: root

    property QtObject model: null

    property NavigationPanel navigationPanel: null
    property int navigationRowStart: 1

    implicitHeight: contentColumn.height
    width: parent.width

    Column {
        id: contentColumn

        height: childrenRect.height
        width: parent.width

        spacing: 12

        visible: root.model ? root.model.areSettingsAvailable : false

        Item {
            height: childrenRect.height
            width: parent.width

            CheckBox {
                id: barreModeCheckBox

                anchors.left: parent.left
                anchors.right: parent.horizontalCenter
                anchors.rightMargin: 2

                enabled: root.model ? !root.model.isMultipleDotsModeOn : false
                checked: root.model && enabled ? root.model.isBarreModeOn : false
                text: qsTrc("inspector", "Barré")

                navigation.name: "BarreModeCheckBox"
                navigation.panel: root.navigationPanel
                navigation.row: root.navigationRowStart + 1

                onClicked: { root.model.isBarreModeOn = !checked }
            }

            CheckBox {
                id: multipleDotsModeCheckBox

                anchors.left: parent.horizontalCenter
                anchors.leftMargin: 2
                anchors.right: parent.right

                enabled: root.model ? !root.model.isBarreModeOn : false
                checked: root.model && enabled ? root.model.isMultipleDotsModeOn : false
                text: qsTrc("inspector", "Multiple dots")

                navigation.name: "MultipleDotsCheckBox"
                navigation.panel: root.navigationPanel
                navigation.row: root.navigationRowStart + 2

                onClicked: { root.model.isMultipleDotsModeOn = !checked }
            }
        }

        Column {
            width: parent.width

            spacing: 8

            StyledTextLabel {
                id: markerTypeLabel
                width: parent.width
                text: qsTrc("inspector", "Marker type")
                horizontalAlignment: Text.AlignLeft
            }

            RadioButtonGroup {
                id: lineStyleButtonList

                height: 30
                width: parent.width

                enabled: root.model ? !root.model.isBarreModeOn : false

                model: [
                    { iconRole: IconCode.FRETBOARD_MARKER_CIRCLE_FILLED, typeRole: FretDiagramTypes.DOT_NORMAL, titleRole: qsTrc("inspector", "Normal") },
                    { iconRole: IconCode.CLOSE_X_ROUNDED, typeRole: FretDiagramTypes.DOT_CROSS, titleRole: qsTrc("inspector", "Cross") },
                    { iconRole: IconCode.STOP, typeRole: FretDiagramTypes.DOT_SQUARE, titleRole: qsTrc("inspector", "Square") },
                    { iconRole: IconCode.FRETBOARD_MARKER_TRIANGLE, typeRole: FretDiagramTypes.DOT_TRIANGLE, titleRole: qsTrc("inspector", "Triangle") }
                ]

                delegate: FlatRadioButton {
                    iconCode: modelData["iconRole"]
                    checked: root.model ? root.model.currentFretDotType === modelData["typeRole"] : false

                    navigation.name: "LineStyleGroup"
                    navigation.panel: root.navigationPanel
                    navigation.row: root.navigationRowStart + 3 + index
                    navigation.accessible.name: markerTypeLabel.text + " " + modelData["titleRole"]

                    onToggled: {
                        root.model.currentFretDotType = modelData["typeRole"]
                    }
                }
            }
        }
    }
}

