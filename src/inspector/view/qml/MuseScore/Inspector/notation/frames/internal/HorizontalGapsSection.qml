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

import MuseScore.Inspector 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0

import "../../../common"

Item {
    id: root

    property PropertyItem leftGap: null
    property PropertyItem rightGap: null

    height: childrenRect.height
    width: parent.width

    InspectorPropertyView {
        anchors.left: parent.left
        anchors.right: parent.horizontalCenter
        anchors.rightMargin: 4

        titleText: qsTrc("inspector", "Left gap")
        propertyItem: root.leftGap

        IncrementalPropertyControl {
            icon: IconCode.LEFT_GAP

            enabled: root.leftGap ? lroot.eftGap.isEnabled : false
            isIndeterminate: root.leftGap && enabled ? root.leftGap.isUndefined : false
            currentValue: root.leftGap ? root.leftGap.value : 0

            onValueEdited: { root.leftGap.value = newValue }
        }
    }

    InspectorPropertyView {
        anchors.left: parent.horizontalCenter
        anchors.leftMargin: 4
        anchors.right: parent.right

        titleText: qsTrc("inspector", "Right gap")
        propertyItem: root.rightGap

        IncrementalPropertyControl {
            icon: IconCode.RIGHT_GAP

            enabled: root.rightGap ? root.rightGap.isEnabled : false
            isIndeterminate: root.rightGap && enabled ? root.rightGap.isUndefined : false
            currentValue: root.rightGap ? root.rightGap.value : 0

            onValueEdited: { root.rightGap.value = newValue }
        }
    }
}
