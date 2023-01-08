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

InspectorPropertyView {
    id: root

    property PropertyItem horizontalOffset: null
    property PropertyItem verticalOffset: null

    property alias horizontalOffsetControl: horizontalOffsetControl
    property alias verticalOffsetControl: verticalOffsetControl
    property bool isVerticalOffsetAvailable: true

    titleText: qsTrc("inspector", "Offset")

    navigationName: "OffsetSection"
    navigationRowEnd: verticalOffsetControl.navigation.row

    enabled: horizontalOffset || verticalOffset
    visible: horizontalOffset || verticalOffset

    isStyled: (horizontalOffset && horizontalOffset.isStyled)
              || (verticalOffset && verticalOffset.isStyled)
    isModified: (horizontalOffset && horizontalOffset.isModified)
                || (verticalOffset && verticalOffset.isModified)

    onRequestApplyToStyle: {
        if (horizontalOffset && horizontalOffset.isStyled) {
            horizontalOffset.applyToStyle()
        }

        if (verticalOffset && verticalOffset.isStyled) {
            verticalOffset.applyToStyle()
        }
    }

    onRequestResetToDefault: {
        if (horizontalOffset) {
            horizontalOffset.resetToDefault()
        }

        if (verticalOffset) {
            verticalOffset.resetToDefault()
        }
    }

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

            enabled: root.horizontalOffset && root.horizontalOffset.isEnabled
            isIndeterminate: !root.horizontalOffset || root.horizontalOffset.isUndefined
            currentValue: root.horizontalOffset ? root.horizontalOffset.value : 0

            onValueEditingFinished: function(newValue) {
                if (root.horizontalOffset) {
                    root.horizontalOffset.value = newValue
                }
            }
        }

        IncrementalPropertyControl {
            id: verticalOffsetControl

            width: parent.width / 2 - row.spacing / 2

            navigation.name: "VerticalOffsetControl"
            navigation.panel: root.navigationPanel
            navigation.row: root.navigationRowStart + 2
            navigation.accessible.name: root.titleText + " " + qsTrc("inspector", "Vertical") + " " + currentValue

            icon: IconCode.VERTICAL

            enabled: root.isVerticalOffsetAvailable && root.verticalOffset && root.verticalOffset.isEnabled
            isIndeterminate: !root.verticalOffset || root.verticalOffset.isUndefined
            currentValue: root.verticalOffset ? root.verticalOffset.value : 0

            onValueEditingFinished: function(newValue) {
                if (root.verticalOffset) {
                    root.verticalOffset.value = newValue
                }
            }
        }
    }
}
