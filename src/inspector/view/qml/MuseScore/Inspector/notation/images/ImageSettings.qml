/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import MuseScore.Inspector 1.0

import "../../common"

Column {
    id: root

    property QtObject model: null

    property NavigationPanel navigationPanel: null
    property int navigationRowStart: 1

    objectName: "ImageSettings"

    spacing: 12

    function focusOnFirst() {
        heightControl.focusOnFirst()
    }

    Item {
        height: childrenRect.height
        width: parent.width

        SpinBoxPropertyView {
            id: heightControl

            anchors.left: parent.left
            anchors.right: lockButton.left
            anchors.rightMargin: 6

            titleText: qsTrc("inspector", "Image height")
            propertyItem: root.model ? root.model.height : null

            icon: IconCode.VERTICAL
            measureUnitsSymbol: staffSpaceUnitsCheckbox.checked ? qsTrc("global", "sp") : qsTrc("global", "mm")

            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigationRowStart + 1
        }

        FlatToggleButton {
            id: lockButton

            anchors.horizontalCenter: parent.horizontalCenter

            height: 20
            width: 20

            y: heightControl.spinBox.y + (heightControl.spinBox.height - height) / 2

            icon: checked ? IconCode.LOCK_CLOSED : IconCode.LOCK_OPEN

            navigation.name: "Lock"
            navigation.panel: root.navigationPanel
            navigation.row: heightControl.navigationRowEnd + 1
            navigation.accessible.name: qsTrc("inspector", "Lock")

            checked: root.model ? root.model.isAspectRatioLocked.value : false
            onToggled: {
                root.model.isAspectRatioLocked.value = !root.model.isAspectRatioLocked.value
            }
        }

        SpinBoxPropertyView {
            id: imageWidthSection
            anchors.left: lockButton.right
            anchors.leftMargin: 6
            anchors.right: parent.right

            titleText: qsTrc("inspector", "Image width")
            propertyItem: root.model ? root.model.width : null

            icon: IconCode.HORIZONTAL
            iconMode: IncrementalPropertyControl.Right
            measureUnitsSymbol: staffSpaceUnitsCheckbox.checked ? qsTrc("global", "sp") : qsTrc("global", "mm")

            navigationPanel: root.navigationPanel
            navigationRowStart: lockButton.navigation.row + 1
        }
    }

    SeparatorLine { anchors.margins: -12 }

    PropertyCheckBox {
        text: qsTrc("inspector", "Scale to frame size")
        propertyItem: root.model ? root.model.shouldScaleToFrameSize : null

        navigation.name: "ScaleToFrameSizeCheckBox"
        navigation.panel: root.navigationPanel
        navigation.row: imageWidthSection.navigationRowEnd + 1
    }

    PropertyCheckBox {
        id: staffSpaceUnitsCheckbox
        text: qsTrc("inspector", "Use staff space units")
        propertyItem: root.model ? root.model.isSizeInSpatiums : null

        navigation.name: "UseStaffSpaceUnitsCheckBox"
        navigation.panel: root.navigationPanel
        navigation.row: imageWidthSection.navigationRowEnd + 2
    }
}
