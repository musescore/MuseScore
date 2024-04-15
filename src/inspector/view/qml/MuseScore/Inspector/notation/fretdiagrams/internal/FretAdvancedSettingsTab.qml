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
import MuseScore.Inspector 1.0
import Muse.UiComponents 1.0

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

        width: parent.width

        spacing: 12

        Item {
            height: childrenRect.height
            width: parent.width

            SpinBoxPropertyView {
                id: scaleSection
                anchors.left: parent.left
                anchors.right: parent.horizontalCenter
                anchors.rightMargin: 2

                titleText: qsTrc("inspector", "Scale")
                propertyItem: root.model ? root.model.scale : null

                measureUnitsSymbol: "%"
                step: 1
                decimals: 0
                maxValue: 300
                minValue: 1

                navigationName: "Scale"
                navigationPanel: root.navigationPanel
                navigationRowStart: root.navigationRowStart
            }

            SpinBoxPropertyView {
                id: stringsSection
                anchors.left: parent.horizontalCenter
                anchors.leftMargin: 2
                anchors.right: parent.right

                titleText: qsTrc("inspector", "Strings")
                propertyItem: root.model ? root.model.stringsCount : null

                step: 1
                decimals: 0
                maxValue: 12
                minValue: 1

                navigationName: "Strings"
                navigationPanel: root.navigationPanel
                navigationRowStart: scaleSection.navigationRowEnd + 1
            }
        }

        Item {
            height: childrenRect.height
            width: parent.width

            SpinBoxPropertyView {
                id: visibleFrets
                anchors.left: parent.left
                anchors.right: parent.horizontalCenter
                anchors.rightMargin: 2

                titleText: qsTrc("inspector", "Visible frets")
                propertyItem: root.model ? root.model.fretsCount : null

                step: 1
                decimals: 0
                maxValue: 24
                minValue: 1

                navigationName: "VisibleFrets"
                navigationPanel: root.navigationPanel
                navigationRowStart: stringsSection.navigationRowEnd + 1
            }

            SpinBoxPropertyView {
                id: fretNumber
                anchors.left: parent.horizontalCenter
                anchors.leftMargin: 2
                anchors.right: parent.right

                titleText: qsTrc("inspector", "Fret number")
                propertyItem: root.model ? root.model.fretNumber : null

                step: 1
                decimals: 0
                maxValue: 24
                minValue: 1

                navigationName: "FretNumber"
                navigationPanel: root.navigationPanel
                navigationRowStart: visibleFrets.navigationRowEnd + 1
            }
        }

        PropertyCheckBox {
            id: multipleDotsCheckbox
            anchors.left: parent.left
            anchors.right: parent.horizontalCenter
            anchors.rightMargin: 2

            text: qsTrc("inspector", "Show nut")
            propertyItem: root.model ? root.model.isNutVisible : false

            navigation.name: "MultipleDotsCheckBox"
            navigation.panel: root.navigationPanel
            navigation.row: fretNumber.navigationRowEnd + 1
        }

        FlatRadioButtonGroupPropertyView {
            id: orientationSection
            titleText: qsTrc("inspector", "Orientation")
            propertyItem: root.model ? root.model.orientation : null

            model: [
                { text: qsTrc("inspector", "Vertical"), value: FretDiagramTypes.ORIENTATION_VERTICAL },
                { text: qsTrc("inspector", "Horizontal"), value: FretDiagramTypes.ORIENTATION_HORIZONTAL }
            ]

            navigationPanel: root.navigationPanel
            navigationRowStart: multipleDotsCheckbox.navigation.row + 1
        }

        PlacementSection {
            id: placementSection
            propertyItem: root.model ? root.model.placement : null

            navigationPanel: root.navigationPanel
            navigationRowStart: orientationSection.navigationRowEnd + 1
        }
    }
}
