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

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import MuseScore.Inspector 1.0

InspectorPropertyView {
    id: root

    property alias horizontalOffsetControl: horizontalOffsetControl
    property alias verticalOffsetControl: verticalOffsetControl
    property bool isVerticalOffsetAvailable: true

    titleText: qsTrc("inspector", "Offset")

    navigationName: "OffsetSection"
    navigationRowEnd: verticalOffsetControl.navigation.row

    Row {
        id: row

        height: childrenRect.height
        width: parent.width

        spacing: 8

        IncrementalPropertyControl {
            id: horizontalOffsetControl

            width: parent.width / 2 - row.spacing / 2

            navigation.name: "HorizontalOffsetControl"
            navigation.panel: root.navigationPanel
            navigation.row: root.navigationRowStart + 1
            navigation.accessible.name: root.titleText + " " + qsTrc("inspector", "Horizontal") + " " + currentValue

            icon: IconCode.HORIZONTAL

            isIndeterminate: root.propertyItem && enabled ? root.propertyItem.isUndefined : false
            currentValue: root.propertyItem ? root.propertyItem.x : 0

            onValueEditingFinished: function(newValue) {
                root.propertyItem.x = newValue
            }
        }

        IncrementalPropertyControl {
            id: verticalOffsetControl

            enabled: root.isVerticalOffsetAvailable

            width: parent.width / 2 - row.spacing / 2

            navigation.name: "VerticalOffsetControl"
            navigation.panel: root.navigationPanel
            navigation.row: root.navigationRowStart + 2
            navigation.accessible.name: root.titleText + " " + qsTrc("inspector", "Vertical") + " " + currentValue

            icon: IconCode.VERTICAL

            isIndeterminate: root.propertyItem && enabled ? root.propertyItem.isUndefined : false
            currentValue: root.propertyItem ? root.propertyItem.y : 0

            onValueEditingFinished: function(newValue) {
                root.propertyItem.y = newValue
            }
        }
    }
}
