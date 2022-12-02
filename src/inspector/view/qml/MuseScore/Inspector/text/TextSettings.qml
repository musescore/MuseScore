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

import "../common"

Column {
    id: root

    property QtObject model: null

    property NavigationPanel navigationPanel: null
    property int navigationRowStart: 0

    width: parent.width

    spacing: 12

    function forceFocusIn() {
        matchStaffSize.navigation.requestActive()
    }

    Item {
        height: childrenRect.height
        width: parent.width

        CheckBoxPropertyView {
            id: matchStaffSize
            anchors.left: parent.left
            anchors.right: subscriptOptionsButtonList.left
            anchors.rightMargin: 8
            anchors.verticalCenter: subscriptOptionsButtonList.verticalCenter

            navigation.name: "Match staff size"
            navigation.panel: root.navigationPanel
            navigation.row: root.navigationRowStart + 1

            text: qsTrc("inspector", "Match staff size")
            propertyItem: root.model ? root.model.isSizeSpatiumDependent : null
        }

        RadioButtonGroup {
            id: subscriptOptionsButtonList

            property int navigationRowStart: matchStaffSize.navigation.row + 1
            property int navigationRowEnd: navigationRowStart + count

            anchors.right: parent.right
            height: 30

            model: [
                { iconRole: IconCode.TEXT_SUBSCRIPT, typeRole: TextTypes.TEXT_SUBSCRIPT_BOTTOM, titleRole: qsTrc("inspector", "Subscript") },
                { iconRole: IconCode.TEXT_SUPERSCRIPT, typeRole: TextTypes.TEXT_SUBSCRIPT_TOP, titleRole: qsTrc("inspector", "Superscript") }
            ]

            delegate: FlatRadioButton {
                navigation.name: "ScriptOptions" + model.index
                navigation.panel: root.navigationPanel
                navigation.row: subscriptOptionsButtonList.navigationRowStart + model.index
                navigation.accessible.name: modelData["titleRole"]

                width: 30
                transparent: true

                iconCode: modelData["iconRole"]
                checked: root.model && !root.model.textScriptAlignment.isUndefined ? root.model.textScriptAlignment.value === modelData["typeRole"]
                                                                                   : false
                onClicked: {
                    if (root.model.textScriptAlignment.value === modelData["typeRole"]) {
                        root.model.textScriptAlignment.value = TextTypes.TEXT_SUBSCRIPT_NORMAL
                    } else {
                        root.model.textScriptAlignment.value = modelData["typeRole"]
                    }
                }
            }
        }
    }

    FlatRadioButtonGroupPropertyView {
        id: frameSection
        titleText: qsTrc("inspector", "Frame")
        propertyItem: root.model ? root.model.frameType : null

        navigationName: "FrameMenu"
        navigationPanel: root.navigationPanel
        navigationRowStart: subscriptOptionsButtonList.navigationRowEnd + 1

        model: [
            { text: qsTrc("inspector", "None"), value: TextTypes.FRAME_TYPE_NONE, titleRole: qsTrc("inspector", "None") },
            { iconCode: IconCode.FRAME_SQUARE, value: TextTypes.FRAME_TYPE_SQUARE, titleRole: qsTrc("inspector", "Rectangle") },
            { iconCode: IconCode.FRAME_CIRCLE, value: TextTypes.FRAME_TYPE_CIRCLE, titleRole: qsTrc("inspector", "Circle") }
        ]
    }

    Item {
        height: childrenRect.height
        width: parent.width

        ColorSection {
            id: borderColorSection
            anchors.left: parent.left
            anchors.right: parent.horizontalCenter
            anchors.rightMargin: 2

            navigationName: "BorderColorMenu"
            navigationPanel: root.navigationPanel
            navigationRowStart: frameSection.navigationRowEnd + 1

            visible: root.model ? root.model.frameBorderColor.isEnabled : false
            height: visible ? implicitHeight : 0

            titleText: qsTrc("inspector", "Border")
            propertyItem: root.model ? root.model.frameBorderColor : null
        }

        ColorSection {
            id: highlightColorSection
            anchors.left: parent.horizontalCenter
            anchors.leftMargin: 2
            anchors.right: parent.right

            navigationName: "HighlightColorMenu"
            navigationPanel: root.navigationPanel
            navigationRowStart: borderColorSection.navigationRowEnd + 1

            visible: root.model ? root.model.frameFillColor.isEnabled : false
            height: visible ? implicitHeight : 0

            titleText: qsTrc("inspector", "Fill color")
            propertyItem: root.model ? root.model.frameFillColor : null
        }
    }

    Item {
        height: childrenRect.height
        width: parent.width

        SpinBoxPropertyView {
            id: thicknessSection
            anchors.left: parent.left
            anchors.right: parent.horizontalCenter
            anchors.rightMargin: 2

            navigationName: "Thickness"
            navigationPanel: root.navigationPanel
            navigationRowStart: highlightColorSection.navigationRowEnd + 1

            visible: root.model ? root.model.frameThickness.isEnabled : false
            height: visible ? implicitHeight : 0

            titleText: qsTrc("inspector", "Thickness")
            propertyItem: root.model ? root.model.frameThickness : null

            step: 0.1
            minValue: 0.1
            maxValue: 5
        }

        SpinBoxPropertyView {
            id: marginSection
            anchors.left: parent.horizontalCenter
            anchors.leftMargin: 2
            anchors.right: parent.right

            navigationName: "Margin"
            navigationPanel: root.navigationPanel
            navigationRowStart: thicknessSection.navigationRowEnd + 1

            visible: root.model ? root.model.frameMargin.isEnabled : false
            height: visible ? implicitHeight : 0

            titleText: qsTrc("inspector", "Margin")
            propertyItem: root.model ? root.model.frameMargin : null

            step: 0.1
            minValue: 0
            maxValue: 5
        }
    }

    SpinBoxPropertyView {
        id: cornerRadiusSection
        anchors.left: parent.left
        anchors.right: parent.horizontalCenter
        anchors.rightMargin: 2

        navigationName: "Corner radius"
        navigationPanel: root.navigationPanel
        navigationRowStart: marginSection.navigationRowEnd + 1

        visible: root.model ? root.model.frameCornerRadius.isEnabled : false
        height: visible ? implicitHeight : 0

        titleText: qsTrc("inspector", "Corner radius")
        propertyItem: root.model ? root.model.frameCornerRadius : null

        step: 0.1
        minValue: 0
        maxValue: 5
    }

    SeparatorLine { anchors.margins: -12 }

    DropdownPropertyView {
        id: textStyleSection
        titleText: qsTrc("inspector", "Text style")
        propertyItem: root.model ? root.model.textType : null

        navigationName: "Text style"
        navigationPanel: root.navigationPanel
        navigationRowStart: cornerRadiusSection.navigationRowEnd + 1

        model: root.model ? root.model.textStyles : []
    }

    PlacementSection {
        id: textPlacementSection
        propertyItem: root.model ? root.model.textPlacement : null

        navigationPanel: root.navigationPanel
        navigationRowStart: textStyleSection.navigationRowEnd + 1
    }

    FlatButton {
        width: parent.width

        navigation.name: "Staff text properties"
        navigation.panel: root.navigationPanel
        navigation.row: textPlacementSection.navigationRowEnd + 1

        text: qsTrc("inspector", "Staff text properties")

        visible: root.model ? root.model.areStaffTextPropertiesAvailable : false

        onClicked: {
            root.model.showStaffTextProperties()
        }
    }
}
