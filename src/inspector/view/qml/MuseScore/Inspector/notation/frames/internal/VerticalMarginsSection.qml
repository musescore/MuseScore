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
import MuseScore.Ui 1.0
import MuseScore.Inspector 1.0

import "../../../common"

Item {
    id: root

    property PropertyItem frameTopMargin: null
    property PropertyItem frameBottomMargin: null

    height: childrenRect.height
    width: parent.width

    InspectorPropertyView {
        anchors.left: parent.left
        anchors.right: parent.horizontalCenter
        anchors.rightMargin: 4

        titleText: qsTrc("inspector", "Top margin")
        propertyItem: root.frameTopMargin

        IncrementalPropertyControl {
            icon: IconCode.TOP_MARGIN

            measureUnitsSymbol: qsTrc("inspector", "mm")

            enabled: root.frameTopMargin ? root.frameTopMargin.isEnabled : false
            isIndeterminate: root.frameTopMargin && enabled ? root.frameTopMargin.isUndefined : false
            currentValue: root.frameTopMargin ? root.frameTopMargin.value : 0

            onValueEdited: { root.frameTopMargin.value = newValue }
        }
    }

    InspectorPropertyView {
        anchors.left: parent.horizontalCenter
        anchors.leftMargin: 4
        anchors.right: parent.right

        titleText: qsTrc("inspector", "Bottom margin")
        propertyItem: root.frameBottomMargin

        IncrementalPropertyControl {
            icon: IconCode.BOTTOM_MARGIN

            measureUnitsSymbol: qsTrc("inspector", "mm")

            enabled: root.frameBottomMargin ? root.frameBottomMargin.isEnabled : false
            isIndeterminate: root.frameBottomMargin && enabled ? root.frameBottomMargin.isUndefined : false
            currentValue: root.frameBottomMargin ? root.frameBottomMargin.value : 0

            onValueEdited: { root.frameBottomMargin.value = newValue }
        }
    }
}
