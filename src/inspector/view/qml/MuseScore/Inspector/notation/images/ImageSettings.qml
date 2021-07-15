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
import "../../common"

Column {
    id: root

    property QtObject model: null

    objectName: "ImageSettings"

    spacing: 12

    Item {
        height: childrenRect.height
        width: parent.width

        InspectorPropertyView {
            id: heightControl

            anchors.left: parent.left
            anchors.right: lockButton.left
            anchors.rightMargin: 6

            titleText: qsTrc("inspector", "Image height")
            propertyItem: root.model ? root.model.height : null

            IncrementalPropertyControl {

                icon: IconCode.VERTICAL
                measureUnitsSymbol: staffSpaceUnitsCheckbox.checked ? qsTrc("inspector", "sp") : qsTrc("inspector", "mm")
                isIndeterminate: root.model ? root.model.height.isUndefined : false
                currentValue: root.model ? root.model.height.value : 0

                onValueEdited: { root.model.height.value = newValue }
            }
        }

        FlatToggleButton {
            id: lockButton

            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: heightControl.verticalCenter

            height: 20
            width: 20

            icon: checked ? IconCode.LOCK_CLOSED : IconCode.LOCK_OPEN

            checked: root.model ? root.model.isAspectRatioLocked.value : false

            onToggled: {
                root.model.isAspectRatioLocked.value = !model.isAspectRatioLocked.value
            }
        }

        InspectorPropertyView {
            anchors.left: lockButton.right
            anchors.leftMargin: 6
            anchors.right: parent.right

            titleText: qsTrc("inspector", "Image width")
            propertyItem: root.model ? root.model.width : null

            IncrementalPropertyControl {
                icon: IconCode.HORIZONTAL
                iconMode: iconModeEnum.right
                measureUnitsSymbol: staffSpaceUnitsCheckbox.checked ? qsTrc("inspector", "sp") : qsTrc("inspector", "mm")
                isIndeterminate: root.model ? root.model.width.isUndefined : false
                currentValue: root.model ? root.model.width.value : 0

                onValueEdited: { root.model.width.value = newValue }
            }
        }
    }

    SeparatorLine { anchors.margins: -10 }

    CheckBox {
        enabled: root.model ? root.model.shouldScaleToFrameSize.isEnabled : false
        isIndeterminate: root.model ? root.model.shouldScaleToFrameSize.isUndefined : false
        checked: root.model && !isIndeterminate ? root.model.shouldScaleToFrameSize.value : false
        text: qsTrc("inspector", "Scale to frame size")

        onClicked: { root.model.shouldScaleToFrameSize.value = !checked }
    }

    CheckBox {
        id: staffSpaceUnitsCheckbox

        isIndeterminate: root.model ? root.model.isSizeInSpatiums.isUndefined : false
        checked: root.model && !isIndeterminate ? root.model.isSizeInSpatiums.value : false
        text: qsTrc("inspector", "Use staff space units")

        onClicked: { root.model.isSizeInSpatiums.value = !checked }
    }
}
