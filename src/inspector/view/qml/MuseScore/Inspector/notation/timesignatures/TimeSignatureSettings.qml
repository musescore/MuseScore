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

    objectName: "TimeSignatureSettings"

    spacing: 12

    function focusOnFirst() {
        horizontalScaleControl.navigation.requestActive()
    }

    InspectorPropertyView {
        id: scaleSection
        height: childrenRect.height

        titleText: qsTrc("inspector", "Scale")
        propertyItem: root.model ? root.model.horizontalScale : null

        navigationPanel: root.navigationPanel
        navigationRowStart: root.navigationRowStart + 1
        navigationRowEnd: verticalScaleControl.navigation.row

        isModified: root.model ? (root.model.horizontalScale.isModified
                                  || root.model.verticalScale.isModified) : false

        onRequestResetToDefault: {
            if (root.model) {
                root.model.horizontalScale.resetToDefault()
                root.model.verticalScale.resetToDefault()
            }
        }

        onRequestApplyToStyle: {
            if (root.model) {
                root.model.horizontalScale.applyToStyle()
                root.model.verticalScale.applyToStyle()
            }
        }

        Item {
            height: childrenRect.height
            width: parent.width

            IncrementalPropertyControl {
                id: horizontalScaleControl

                anchors.left: parent.left
                anchors.right: parent.horizontalCenter
                anchors.rightMargin: 2

                icon: IconCode.HORIZONTAL
                isIndeterminate: root.model ? root.model.horizontalScale.isUndefined : false
                currentValue: root.model ? root.model.horizontalScale.value : 0

                measureUnitsSymbol: "%"
                step: 1
                decimals: 0
                maxValue: 1000
                minValue: 1

                navigation.name: "HorizontalScale"
                navigation.panel: root.navigationPanel
                navigation.row: scaleSection.navigationRowStart + 2
                navigation.accessible.name: scaleSection.titleText + " " + qsTrc("inspector", "Horizontal") + currentValue

                onValueEditingFinished: function(newValue) {
                    root.model.horizontalScale.value = newValue
                }
            }

            IncrementalPropertyControl {
                id: verticalScaleControl

                anchors.left: parent.horizontalCenter
                anchors.leftMargin: 2
                anchors.right: parent.right

                icon: IconCode.VERTICAL
                isIndeterminate: root.model ? root.model.verticalScale.isUndefined : false
                currentValue: root.model ? root.model.verticalScale.value : 0

                measureUnitsSymbol: "%"
                step: 1
                decimals: 0
                maxValue: 1000
                minValue: 1

                navigation.name: "VerticalScale"
                navigation.panel: root.navigationPanel
                navigation.row: scaleSection.navigationRowStart + 3
                navigation.accessible.name: scaleSection.titleText + " " + qsTrc("inspector", "Vertical") + currentValue

                onValueEditingFinished: function(newValue) {
                    root.model.verticalScale.value = newValue
                }
            }
        }
    }

    PropertyCheckBox {
        text: qsTrc("inspector", "Show courtesy time signature on previous system")
        propertyItem: root.model ? root.model.shouldShowCourtesy : null

        navigation.name: "ShowCourtesyCheckBox"
        navigation.panel: root.navigationPanel
        navigation.row: scaleSection.navigationRowEnd + 1
    }

    FlatButton {
        width: parent.width

        text: qsTrc("inspector", "Time signature properties")

        navigation.name: "ChangeButton"
        navigation.panel: root.navigationPanel
        navigation.row: scaleSection.navigationRowEnd + 2

        onClicked: {
            if (root.model) {
                root.model.showTimeSignatureProperties()
            }
        }
    }
}
