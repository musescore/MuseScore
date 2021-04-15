/**
 * // SPDX-License-Identifier: GPL-3.0-only
 * // MuseScore-CLA-applies
 * //=============================================================================
 * //  MuseScore
 * //  Music Composition & Notation
 * //
 * //  Copyright (C) 2021 MuseScore BVBA and others
 * //
 * //  This program is free software: you can redistribute it and/or modify
 * //  it under the terms of the GNU General Public License version 3 as
 * //  published by the Free Software Foundation.
 * //
 * //  This program is distributed in the hope that it will be useful,
 * //  but WITHOUT ANY WARRANTY; without even the implied warranty of
 * //  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * //  GNU General Public License for more details.
 * //
 * //  You should have received a copy of the GNU General Public License
 * //  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * //============================================================================= **/
import QtQuick 2.9
import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0
import "../../../common"

Item {
    id: root

    property QtObject frameLeftMargin: undefined
    property QtObject frameRightMargin: undefined

    height: childrenRect.height
    width: parent.width

    InspectorPropertyView {
        anchors.left: parent.left
        anchors.right: parent.horizontalCenter
        anchors.rightMargin: 4

        titleText: qsTrc("inspector", "Left margin")
        propertyItem: frameLeftMargin

        IncrementalPropertyControl {
            icon: IconCode.LEFT_MARGIN

            measureUnitsSymbol: qsTrc("inspector", "mm")

            enabled: frameLeftMargin ? frameLeftMargin.isEnabled : false
            isIndeterminate: frameLeftMargin && enabled ? frameLeftMargin.isUndefined : false
            currentValue: frameLeftMargin ? frameLeftMargin.value : 0

            onValueEdited: { frameLeftMargin.value = newValue }
        }
    }

    InspectorPropertyView {
        anchors.left: parent.horizontalCenter
        anchors.leftMargin: 4
        anchors.right: parent.right

        titleText: qsTrc("inspector", "Right margin")
        propertyItem: frameRightMargin

        IncrementalPropertyControl {
            icon: IconCode.RIGHT_MARGIN

            measureUnitsSymbol: qsTrc("inspector", "mm")

            enabled: frameRightMargin ? frameRightMargin.isEnabled : false
            isIndeterminate: frameRightMargin && enabled ? frameRightMargin.isUndefined : false
            currentValue: frameRightMargin ? frameRightMargin.value : 0

            onValueEdited: { frameRightMargin.value = newValue }
        }
    }
}
