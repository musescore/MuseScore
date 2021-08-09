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
import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Inspector 1.0
import "../../../common"

Column {
    id: root

    property QtObject horizontalOffset: undefined
    property QtObject verticalOffset: undefined
    property bool isSnappedToGrid: false

    property NavigationPanel navigationPanel: null
    property int navigationRowOffset: 0

    signal snapToGridToggled(var snap)
    signal configureGridRequested()

    height: implicitHeight
    width: parent.width

    spacing: 16

    function navigationRow(r) {
        return root.navigationRowOffset + r
    }

    InspectorPropertyView {
        titleText: qsTrc("inspector", "Offset")
        propertyItem: root.horizontalOffset

        navigation.name: "OffsetMenu"
        navigation.panel: root.navigationPanel
        navigation.row: root.navigationRow(1)

        Item {
            height: childrenRect.height
            width: parent.width

            IncrementalPropertyControl {
                anchors.left: parent.left
                anchors.right: parent.horizontalCenter
                anchors.rightMargin: 4

                navigation.name: "HorizontalOffsetValue"
                navigation.panel: root.navigationPanel
                navigation.row: root.navigationRow(2)

                icon: IconCode.HORIZONTAL

                enabled: root.horizontalOffset ? root.horizontalOffset.isEnabled : false
                isIndeterminate: root.horizontalOffset && enabled ? root.horizontalOffset.isUndefined : false
                currentValue: root.horizontalOffset ? root.horizontalOffset.value : 0

                onValueEdited: { root.horizontalOffset.value = newValue }
            }

            IncrementalPropertyControl {
                anchors.left: parent.horizontalCenter
                anchors.leftMargin: 4
                anchors.right: parent.right

                navigation.name: "VerticalOffsetValue"
                navigation.panel: root.navigationPanel
                navigation.row: root.navigationRow(3)

                icon: IconCode.VERTICAL

                enabled: root.verticalOffset ? root.verticalOffset.isEnabled : false
                isIndeterminate: root.verticalOffset && enabled ? root.verticalOffset.isUndefined : false
                currentValue: root.verticalOffset ? root.verticalOffset.value : 0

                onValueEdited: { root.verticalOffset.value = newValue }
            }
        }
    }

    CheckBox {
        id: snapToGridCheckbox

        navigation.name: "Snap to grid"
        navigation.panel: root.navigationPanel
        navigation.row: root.navigationRow(4)

        text: qsTrc("inspector", "Snap to grid")

        checked: isSnappedToGrid

        onClicked: { root.snapToGridToggled(!checked) }
    }

    FlatButton {
        width: parent.width

        navigation.name: "Configure grid"
        navigation.panel: root.navigationPanel
        navigation.row: root.navigationRow(5)

        text: qsTrc("inspector", "Configure grid")

        visible: snapToGridCheckbox.checked

        onClicked: {
            root.configureGridRequested()
        }
    }
}
