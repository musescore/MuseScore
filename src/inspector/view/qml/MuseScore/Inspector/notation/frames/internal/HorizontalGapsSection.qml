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
import MuseScore.Inspector 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0
import "../../../common"

Item {
    id: root

    property QtObject leftGap: undefined
    property QtObject rightGap: undefined

    height: childrenRect.height
    width: parent.width

    InspectorPropertyView {
        anchors.left: parent.left
        anchors.right: parent.horizontalCenter
        anchors.rightMargin: 4

        titleText: qsTrc("inspector", "Left gap")
        propertyItem: leftGap

        IncrementalPropertyControl {
            icon: IconCode.LEFT_GAP

            enabled: leftGap ? leftGap.isEnabled : false
            isIndeterminate: leftGap && enabled ? leftGap.isUndefined : false
            currentValue: leftGap ? leftGap.value : 0

            onValueEdited: { leftGap.value = newValue }
        }
    }

    InspectorPropertyView {
        anchors.left: parent.horizontalCenter
        anchors.leftMargin: 4
        anchors.right: parent.right

        titleText: qsTrc("inspector", "Right gap")
        propertyItem: rightGap

        IncrementalPropertyControl {
            icon: IconCode.RIGHT_GAP

            enabled: rightGap ? rightGap.isEnabled : false
            isIndeterminate: rightGap && enabled ? rightGap.isUndefined : false
            currentValue: rightGap ? rightGap.value : 0

            onValueEdited: { rightGap.value = newValue }
        }
    }
}
