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

        PropertyCheckBox {
            id: matchStaffSize
            anchors.left: parent.left
            anchors.right: subscriptOptionsButtonList.left
            anchors.rightMargin: 8
            anchors.verticalCenter: subscriptOptionsButtonList.verticalCenter

            navigation.name: "Scale with staff size"
            navigation.panel: root.navigationPanel
            navigation.row: root.navigationRowStart + 1

            text: qsTrc("inspector", "Scale with staff size")
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

    FrameSettings {
        id: frameSettings
        visible: root.model ? !root.model.isDynamicSpecificSettings : false
        height: visible ? implicitHeight : 0

        navigationPanel: root.navigationPanel
        navigationRowStart: subscriptOptionsButtonList.navigationRowEnd + 1

        frameType: root.model ? root.model.frameType : null
        frameBorderColor: root.model ? root.model.frameBorderColor : null
        frameFillColor: root.model ? root.model.frameFillColor : null
        frameThickness: root.model ? root.model.frameThickness : null
        frameMargin: root.model ? root.model.frameMargin : null
        frameCornerRadius: root.model ? root.model.frameCornerRadius : null
    }

    SeparatorLine { anchors.margins: -12 }

    SpinBoxPropertyView {
        id: textLineSpacingSection
        anchors.left: parent.left
        anchors.right: parent.horizontalCenter
        anchors.rightMargin: 2

        navigationName: "Line Spacing"
        navigationPanel: root.navigationPanel
        navigationRowStart: frameSettings.navigationRowEnd + 1

        titleText: qsTrc("inspector", "Line spacing")
        //: Stands for "Lines". Used for text line spacing controls, for example.
        measureUnitsSymbol: qsTrc("global", "li")
        propertyItem: root.model ? root.model.textLineSpacing : null

        decimals: 2
        step: 0.1
        minValue: 0
        maxValue: 10
    }

    SeparatorLine {
        visible: root.model ? !root.model.isDynamicSpecificSettings : false
        anchors.margins: -12
    }

    DropdownPropertyView {
        id: textStyleSection
        titleText: qsTrc("inspector", "Text style")
        propertyItem: root.model ? root.model.textType : null

        navigationName: "Text style"
        navigationPanel: root.navigationPanel
        navigationRowStart: textLineSpacingSection.navigationRowEnd + 1

        visible: root.model ? !root.model.isDynamicSpecificSettings : false
        height: visible ? implicitHeight : 0

        model: root.model ? root.model.textStyles : []
    }

    PlacementSection {
        id: textPlacementSection
        propertyItem: root.model ? root.model.textPlacement : null

        visible: root.model ? !root.model.isDynamicSpecificSettings : false
        height: visible ? implicitHeight : 0

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
