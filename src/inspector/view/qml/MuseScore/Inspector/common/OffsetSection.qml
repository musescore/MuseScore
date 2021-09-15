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
import QtQuick.Layouts 1.15

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0

InspectorPropertyView {
    id: root

    property QtObject horizontalOffset: null
    property QtObject verticalOffset: null

    property alias horizontalOffsetControl: horizontalOffsetControl
    property alias verticalOffsetControl: verticalOffsetControl

    property NavigationPanel navigationPanel: null

    property int navigationRowStart: 0
    readonly property alias navigationRowEnd: verticalOffsetControl.navigation.row

    titleText: qsTrc("inspector", "Offset")
    propertyItem: root.horizontalOffset

    navigation.name: "OffsetSection"
    navigation.panel: root.navigationPanel
    navigation.row: prv.navigationRow(1)

    QtObject {
        id: prv

        function navigationRow(r) {
            return root.navigationRowStart + r
        }
    }

    RowLayout {
        height: childrenRect.height
        width: parent.width

        spacing: 8

        IncrementalPropertyControl {
            id: horizontalOffsetControl

            Layout.fillWidth: true

            navigation.name: "HorizontalOffsetControl"
            navigation.panel: root.navigationPanel
            navigation.row: prv.navigationRow(2)

            icon: IconCode.HORIZONTAL

            enabled: Boolean(root.horizontalOffset) && root.horizontalOffset.isEnabled
            visible: Boolean(root.horizontalOffset)

            isIndeterminate: root.horizontalOffset && enabled ? root.horizontalOffset.isUndefined : false
            currentValue: root.horizontalOffset ? root.horizontalOffset.value : 0

            onValueEdited: {
                root.horizontalOffset.value = newValue
            }
        }

        IncrementalPropertyControl {
            id: verticalOffsetControl

            Layout.fillWidth: true

            navigation.name: "VerticalOffsetControl"
            navigation.panel: root.navigationPanel
            navigation.row: prv.navigationRow(3)

            icon: IconCode.VERTICAL

            enabled: Boolean(root.verticalOffset) && root.verticalOffset.isEnabled
            visible: Boolean(root.verticalOffset)

            isIndeterminate: root.verticalOffset && enabled ? root.verticalOffset.isUndefined : false
            currentValue: root.verticalOffset ? root.verticalOffset.value : 0

            onValueEdited: {
                root.verticalOffset.value = newValue
            }
        }
    }
}
