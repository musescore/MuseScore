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
import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0
import "../../../common"

Item {
    id: root

    property QtObject frameTopMargin: undefined
    property QtObject frameBottomMargin: undefined

    height: childrenRect.height
    width: parent.width

    InspectorPropertyView {
        anchors.left: parent.left
        anchors.right: parent.horizontalCenter
        anchors.rightMargin: 4

        titleText: qsTrc("inspector", "Top margin")
        propertyItem: frameTopMargin

        IncrementalPropertyControl {
            icon: IconCode.TOP_MARGIN

            measureUnitsSymbol: qsTrc("inspector", "mm")

            enabled: frameTopMargin ? frameTopMargin.isEnabled : false
            isIndeterminate: frameTopMargin && enabled ? frameTopMargin.isUndefined : false
            currentValue: frameTopMargin ? frameTopMargin.value : 0

            onValueEdited: { frameTopMargin.value = newValue }
        }
    }

    InspectorPropertyView {
        anchors.left: parent.horizontalCenter
        anchors.leftMargin: 4
        anchors.right: parent.right

        titleText: qsTrc("inspector", "Bottom margin")
        propertyItem: frameBottomMargin

        IncrementalPropertyControl {
            icon: IconCode.BOTTOM_MARGIN

            measureUnitsSymbol: qsTrc("inspector", "mm")

            enabled: frameBottomMargin ? frameBottomMargin.isEnabled : false
            isIndeterminate: frameBottomMargin && enabled ? frameBottomMargin.isUndefined : false
            currentValue: frameBottomMargin ? frameBottomMargin.value : 0

            onValueEdited: { frameBottomMargin.value = newValue }
        }
    }
}
