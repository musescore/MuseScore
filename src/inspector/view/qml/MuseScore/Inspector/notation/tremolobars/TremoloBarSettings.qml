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

Item {
    id: root

    property QtObject model: null

    property NavigationPanel navigationPanel: null
    property int navigationRowStart: 1

    objectName: "TremoloBarSettings"

    implicitHeight: content.implicitHeight

    function focusOnFirst() {
        tremoloBarTypeSection.focusOnFirst()
    }

    Column {
        id: content

        width: parent.width

        spacing: 12

        DropdownPropertyView {
            id: tremoloBarTypeSection
            visible: root.model ? root.model.areSettingsAvailable : false

            titleText: qsTrc("inspector", "Tremolo bar type")
            propertyItem: root.model ? root.model.type : null

            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigationRowStart + 1

            model: [
                { text: qsTrc("inspector", "Dip"), value: TremoloBarTypes.TYPE_DIP },
                { text: qsTrc("inspector", "Dive"), value: TremoloBarTypes.TYPE_DIVE },
                { text: qsTrc("inspector", "Release (Up)"), value: TremoloBarTypes.TYPE_RELEASE_UP },
                { text: qsTrc("inspector", "Inverted dip"), value: TremoloBarTypes.TYPE_INVERTED_DIP },
                { text: qsTrc("inspector", "Return"), value: TremoloBarTypes.TYPE_RETURN },
                { text: qsTrc("inspector", "Release (Down)"), value: TremoloBarTypes.TYPE_RELEASE_DOWN },
                { text: qsTrc("inspector", "Custom"), value: TremoloBarTypes.TYPE_CUSTOM }
            ]
        }

        InspectorPropertyView {
            id: tremoloBarCurve
            visible: root.model ? root.model.areSettingsAvailable : false

            titleText: qsTrc("inspector", "Click to add or remove points")
            propertyItem: root.model ? root.model.curve : null

            navigationPanel: root.navigationPanel
            navigationRowStart: tremoloBarTypeSection.navigationRowEnd + 1

            GridCanvas {
                height: 300
                width: parent.width

                pointList: root.model && root.model.curve.isEnabled ? root.model.curve.value : []

                rowCount: 33
                columnCount: 13
                rowSpacing: 8
                columnSpacing: 3
                shouldShowNegativeRows: true

                onCanvasChanged: {
                    if (root.model) {
                        root.model.curve.value = pointList
                    }
                }
            }
        }

        Item {
            height: childrenRect.height
            width: parent.width

            visible: root.model ? root.model.areSettingsAvailable : false

            SpinBoxPropertyView {
                id: lineThicknessSection
                anchors.left: parent.left
                anchors.right: parent.horizontalCenter
                anchors.rightMargin: 2

                titleText: qsTrc("inspector", "Line thickness")
                propertyItem: root.model ? root.model.lineThickness : null

                maxValue: 10
                minValue: 0.1
                step: 0.1
                decimals: 2

                navigationPanel: root.navigationPanel
                navigationRowStart: tremoloBarCurve.navigationRowEnd + 1
            }

            SpinBoxPropertyView {
                anchors.left: parent.horizontalCenter
                anchors.leftMargin: 2
                anchors.right: parent.right

                titleText: qsTrc("inspector", "Scale")
                propertyItem: root.model ? root.model.scale : null

                maxValue: 5
                minValue: 0.1
                step: 0.1
                decimals: 2

                navigationPanel: root.navigationPanel
                navigationRowStart: lineThicknessSection.navigationRowEnd + 1
            }
        }
    }

    StyledTextLabel {
        anchors.fill: parent

        wrapMode: Text.Wrap
        text: qsTrc("inspector", "You have multiple tremolo bars selected. Select a single one to edit its settings.")
        visible: root.model ? !root.model.areSettingsAvailable : false
    }
}
