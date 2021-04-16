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

    property QtObject gapAbove: undefined
    property QtObject gapBelow: undefined

    height: childrenRect.height
    width: parent.width

    InspectorPropertyView {
        anchors.left: parent.left
        anchors.right: parent.horizontalCenter
        anchors.rightMargin: 4

        titleText: qsTrc("inspector", "Gap above")
        propertyItem: gapAbove

        IncrementalPropertyControl {
            icon: IconCode.GAP_ABOVE

            enabled: gapAbove ? gapAbove.isEnabled : false
            isIndeterminate: gapAbove && enabled ? gapAbove.isUndefined : false
            currentValue: gapAbove ? gapAbove.value : 0

            onValueEdited: { gapAbove.value = newValue }
        }
    }

    InspectorPropertyView {
        anchors.left: parent.horizontalCenter
        anchors.leftMargin: 4
        anchors.right: parent.right

        titleText: qsTrc("inspector", "Gap below")
        propertyItem: gapBelow

        IncrementalPropertyControl {
            icon: IconCode.GAP_BELOW

            enabled: gapBelow ? gapBelow.isEnabled : false
            isIndeterminate: gapBelow && enabled ? gapBelow.isUndefined : false
            currentValue: gapBelow ? gapBelow.value : 0

            onValueEdited: { gapBelow.value = newValue }
        }
    }
}
