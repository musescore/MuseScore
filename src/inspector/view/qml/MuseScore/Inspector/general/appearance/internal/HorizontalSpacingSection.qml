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

Item {
    id: root

    property PropertyItem leadingSpace: null
    property PropertyItem barWidth: null

    property NavigationPanel navigationPanel: null
    property int navigationRowOffset: 0

    function navigationRow(r) {
        return root.navigationRowOffset + r
    }

    function focusOnFirst() {
        leadingValue.navigation.requestActive()
    }

    height: childrenRect.height
    width: parent.width

    InspectorPropertyView {
        anchors.left: parent.left
        anchors.right: parent.horizontalCenter
        anchors.rightMargin: 4

        navigation.name: "LeadingMenu"
        navigation.panel: root.navigationPanel
        navigation.row: root.navigationRow(1)

        titleText: qsTrc("inspector", "Leading")
        propertyItem: root.leadingSpace

        IncrementalPropertyControl {
            id: leadingValue
            icon: IconCode.HORIZONTAL

            navigation.name: "LeadingValue"
            navigation.panel: root.navigationPanel
            navigation.row: root.navigationRow(2)

            enabled: root.leadingSpace ? root.leadingSpace.isEnabled : false
            isIndeterminate: root.leadingSpace && enabled ? root.leadingSpace.isUndefined : false
            currentValue: root.leadingSpace ? root.leadingSpace.value : 0

            onValueEdited: { root.leadingSpace.value = newValue }
        }
    }

    InspectorPropertyView {
        anchors.left: parent.horizontalCenter
        anchors.leftMargin: 4
        anchors.right: parent.right

        navigation.name: "Bar width Menu"
        navigation.panel: root.navigationPanel
        navigation.row: root.navigationRow(3)

        titleText: qsTrc("inspector", "Bar width")
        propertyItem: root.barWidth

        IncrementalPropertyControl {
            icon: IconCode.HORIZONTAL

            navigation.name: "Bar width Value"
            navigation.panel: root.navigationPanel
            navigation.row: root.navigationRow(4)

            enabled: root.barWidth ? root.barWidth.isEnabled : false
            isIndeterminate: root.barWidth && enabled ? root.barWidth.isUndefined : false
            currentValue: root.barWidth ? root.barWidth.value : 0

            onValueEdited: { root.barWidth.value = newValue }
        }
    }
}
