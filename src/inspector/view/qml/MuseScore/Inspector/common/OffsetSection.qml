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
import MuseScore.Inspector 1.0

InspectorPropertyView {
    id: root

    property PropertyItem horizontalOffset: null
    property PropertyItem verticalOffset: null

    property alias horizontalOffsetControl: horizontalOffsetControl
    property alias verticalOffsetControl: verticalOffsetControl

    property var onlyOne: (Boolean(horizontalOffset) && !Boolean(verticalOffset))
                          || (Boolean(verticalOffset) && !Boolean(horizontalOffset))

    titleText: qsTrc("inspector", "Offset")
    propertyItem: Boolean(horizontalOffset) ? horizontalOffset : verticalOffset

    navigationName: "OffsetSection"
    navigationRowEnd: verticalOffsetControl.navigation.row

    isModified: (Boolean(horizontalOffset) ? horizontalOffset.isModified : false)
                || (Boolean(verticalOffset) ? verticalOffset.isModified : false)
    visible: Boolean(horizontalOffset) || Boolean(verticalOffset)

    onRequestResetToDefault: {
        if(Boolean(horizontalOffset)) {
            horizontalOffset.resetToDefault()
        }

        if(Boolean(verticalOffset)) {
            verticalOffset.resetToDefault()
        }
    }

    RowLayout {
        id: row

        height: childrenRect.height
        width: parent.width

        spacing: 8

        IncrementalPropertyControl {
            id: horizontalOffsetControl

            Layout.preferredWidth: onlyOne ? parent.width : parent.width / 2 - row.spacing / 2

            navigation.name: "HorizontalOffsetControl"
            navigation.panel: root.navigationPanel
            navigation.row: root.navigationRowStart + 1
            navigation.accessible.name: root.titleText + " " + qsTrc("inspector", "Horizontal") + " " + currentValue

            icon: IconCode.HORIZONTAL

            enabled: Boolean(root.horizontalOffset) && root.horizontalOffset.isEnabled
            visible: Boolean(root.horizontalOffset) && root.horizontalOffset.isVisible

            isIndeterminate: root.horizontalOffset && enabled ? root.horizontalOffset.isUndefined : false
            currentValue: root.horizontalOffset ? root.horizontalOffset.value : 0

            onValueEdited: function(newValue) {
                root.horizontalOffset.value = newValue
            }
        }

        IncrementalPropertyControl {
            id: verticalOffsetControl

            Layout.preferredWidth: onlyOne ? parent.width : parent.width / 2 - row.spacing / 2

            navigation.name: "VerticalOffsetControl"
            navigation.panel: root.navigationPanel
            navigation.row: root.navigationRowStart + 2
            navigation.accessible.name: root.titleText + " " + qsTrc("inspector", "Vertical") + " " + currentValue

            icon: IconCode.VERTICAL

            enabled: Boolean(root.verticalOffset) && root.verticalOffset.isEnabled
            visible: Boolean(root.verticalOffset) && root.verticalOffset.isVisible

            isIndeterminate: root.verticalOffset && enabled ? root.verticalOffset.isUndefined : false
            currentValue: root.verticalOffset ? root.verticalOffset.value : 0

            onValueEdited: function(newValue) {
                root.verticalOffset.value = newValue
            }
        }
    }
}
