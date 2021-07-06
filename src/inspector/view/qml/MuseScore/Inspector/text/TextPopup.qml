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
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.2
import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Inspector 1.0
import "../common"

StyledPopupView {
    id: root

    property QtObject model: null

    contentHeight: contentColumn.childrenRect.height

    navigation.name: "TextPopup"
    navigation.direction: NavigationPanel.Vertical

    onOpened: {
        matchStaffSize.navigation.requestActive()
    }

    Column {
        id: contentColumn

        width: parent.width

        spacing: 12

        Item {
            height: childrenRect.height
            width: parent.width

            CheckBox {
                id: matchStaffSize
                anchors.left: parent.left
                anchors.right: parent.horizontalCenter
                anchors.rightMargin: 2
                anchors.verticalCenter: subscriptOptionsButtonList.verticalCenter

                navigation.name: "Match staff size"
                navigation.panel: root.navigation
                navigation.row: 1

                isIndeterminate: model ? model.isSizeSpatiumDependent.isUndefined : false
                checked: model && !isIndeterminate ? model.isSizeSpatiumDependent.value : false
                text: qsTrc("inspector", "Match staff size")

                onClicked: { model.isSizeSpatiumDependent.value = !checked }
            }

            RadioButtonGroup {
                id: subscriptOptionsButtonList

                anchors.right: parent.right
                height: 30

                model: [
                    { iconRole: IconCode.TEXT_SUBSCRIPT, typeRole: TextTypes.TEXT_SUBSCRIPT_BOTTOM },
                    { iconRole: IconCode.TEXT_SUPERSCRIPT, typeRole: TextTypes.TEXT_SUBSCRIPT_TOP }
                ]

                delegate: FlatRadioButton {

                    width: 30

                    navigation.name: "ScriptOptions" + model.index
                    navigation.panel: root.navigation
                    navigation.row: 2 + model.index

                    normalStateColor: "#00000000"

                    ButtonGroup.group: subscriptOptionsButtonList.radioButtonGroup

                    checked: root.model && !root.model.textScriptAlignment.isUndefined ? root.model.textScriptAlignment.value === modelData["typeRole"]
                                                                                       : false

                    onClicked: {
                        if (root.model.textScriptAlignment.value === modelData["typeRole"]) {
                            root.model.textScriptAlignment.value = TextTypes.TEXT_SUBSCRIPT_NORMAL
                        } else {
                            root.model.textScriptAlignment.value = modelData["typeRole"]
                        }
                    }

                    StyledIconLabel {
                        iconCode: modelData["iconRole"]
                    }
                }
            }
        }

        InspectorPropertyView {
            titleText: qsTrc("inspector", "Frame")
            propertyItem: root.model ? root.model.frameType : null

            navigation.name: "FrameMenu"
            navigation.panel: root.navigation
            navigation.row: 4

            RadioButtonGroup {
                id: frameType

                height: 30
                width: parent.width

                model: [
                    { iconRole: IconCode.NONE, typeRole: TextTypes.FRAME_TYPE_NONE },
                    { iconRole: IconCode.FRAME_SQUARE, typeRole: TextTypes.FRAME_TYPE_SQUARE },
                    { iconRole: IconCode.FRAME_CIRCLE, typeRole: TextTypes.FRAME_TYPE_CIRCLE }
                ]

                delegate: FlatRadioButton {

                    ButtonGroup.group: frameType.radioButtonGroup

                    navigation.name: "FrameType" + model.index
                    navigation.panel: root.navigation
                    navigation.row: 5 + model.index

                    checked: root.model && !root.model.frameType.isUndefined ? root.model.frameType.value === modelData["typeRole"]
                                                                             : false

                    onToggled: {
                        root.model.frameType.value = modelData["typeRole"]
                    }

                    StyledIconLabel {
                        iconCode: modelData["iconRole"]
                    }
                }
            }
        }

        Item {
            height: childrenRect.height
            width: parent.width

            InspectorPropertyView {
                id: frameBorderColorColumn

                navigation.name: "BorderColorMenu"
                navigation.panel: root.navigation
                navigation.row: 9

                anchors.left: parent.left
                anchors.right: parent.horizontalCenter
                anchors.rightMargin: 2

                visible: root.model ? root.model.frameBorderColor.isEnabled : false
                height: visible ? implicitHeight : 0

                titleText: qsTrc("inspector", "Border")
                propertyItem: root.model ? root.model.frameBorderColor : null

                ColorPicker {
                    navigation.name: "BorderColorValue"
                    navigation.panel: root.navigation
                    navigation.row: 10

                    isIndeterminate: root.model  ? root.model.frameBorderColor.isUndefined : false
                    color: root.model && !root.model.frameBorderColor.isUndefined ? root.model.frameBorderColor.value : ui.theme.backgroundPrimaryColor

                    onNewColorSelected: {
                        if (root.model) {
                            root.model.frameBorderColor.value = newColor
                        }
                    }
                }
            }

            InspectorPropertyView {
                id: frameHighlightColorColumn

                anchors.left: parent.horizontalCenter
                anchors.leftMargin: 2
                anchors.right: parent.right

                navigation.name: "HighlightColorMenu"
                navigation.panel: root.navigation
                navigation.row: 11

                visible: root.model ? root.model.frameHighlightColor.isEnabled : false
                height: visible ? implicitHeight : 0

                titleText: qsTrc("inspector", "Highlight")
                propertyItem: root.model ? root.model.frameHighlightColor : null

                ColorPicker {
                    navigation.name: "HighlightColorValue"
                    navigation.panel: root.navigation
                    navigation.row: 12

                    isIndeterminate: root.model ? root.model.frameHighlightColor.isUndefined : false
                    color: root.model && !root.model.frameHighlightColor.isUndefined ? root.model.frameHighlightColor.value : ui.theme.backgroundPrimaryColor

                    onNewColorSelected: {
                        if (root.model) {
                            root.model.frameHighlightColor.value = newColor
                        }
                    }
                }
            }
        }

        Item {
            height: childrenRect.height
            width: parent.width

            InspectorPropertyView {
                id: frameThicknessColumn

                anchors.left: parent.left
                anchors.right: parent.horizontalCenter
                anchors.rightMargin: 2

                navigation.name: "ThicknessMenu"
                navigation.panel: root.navigation
                navigation.row: 13

                visible: root.model ? root.model.frameThickness.isEnabled : false
                height: visible ? implicitHeight : 0

                titleText: qsTrc("inspector", "Thickness")
                propertyItem: root.model ? root.model.frameThickness : null

                IncrementalPropertyControl {
                    iconMode: iconModeEnum.hidden

                    navigation.name: "ThicknessValue"
                    navigation.panel: root.navigation
                    navigation.row: 14

                    isIndeterminate: root.model ? root.model.frameThickness.isUndefined : false
                    currentValue: root.model ? root.model.frameThickness.value : 0

                    step: 0.1
                    minValue: 0.1
                    maxValue: 5

                    onValueEdited: { root.model.frameThickness.value = newValue }
                }
            }

            InspectorPropertyView {
                id: frameMarginColumn

                anchors.left: parent.horizontalCenter
                anchors.leftMargin: 2
                anchors.right: parent.right

                navigation.name: "MarginMenu"
                navigation.panel: root.navigation
                navigation.row: 15

                visible: root.model ? root.model.frameMargin.isEnabled : false
                height: visible ? implicitHeight : 0

                titleText: qsTrc("inspector", "Margin")
                propertyItem: root.model ? root.model.frameMargin : null

                IncrementalPropertyControl {
                    iconMode: iconModeEnum.hidden

                    navigation.name: "MarginValue"
                    navigation.panel: root.navigation
                    navigation.row: 16

                    isIndeterminate: root.model ? root.model.frameMargin.isUndefined : false
                    currentValue: root.model ? root.model.frameMargin.value : 0

                    step: 0.1
                    minValue: 0
                    maxValue: 5

                    onValueEdited: { root.model.frameMargin.value = newValue }
                }
            }
        }

        InspectorPropertyView {
            anchors.left: parent.left
            anchors.right: parent.horizontalCenter
            anchors.rightMargin: 2

            navigation.name: "Corner radius Menu"
            navigation.panel: root.navigation
            navigation.row: 17

            visible: root.model ? root.model.frameCornerRadius.isEnabled : false
            height: visible ? implicitHeight : 0

            titleText: qsTrc("inspector", "Corner radius")
            propertyItem: root.model ? root.model.frameCornerRadius : null

            IncrementalPropertyControl {
                iconMode: iconModeEnum.hidden

                navigation.name: "Corner radius Value"
                navigation.panel: root.navigation
                navigation.row: 18

                isIndeterminate: root.model ? root.model.frameCornerRadius.isUndefined : false
                currentValue: root.model ? root.model.frameCornerRadius.value : 0

                step: 0.1
                minValue: 0
                maxValue: 5

                onValueEdited: { root.model.frameCornerRadius.value = newValue }
            }
        }

        SeparatorLine { anchors.margins: -10 }

        InspectorPropertyView {
            titleText: qsTrc("inspector", "Text style")
            propertyItem: root.model ? root.model.textType : null

            navigation.name: "Text style Menu"
            navigation.panel: root.navigation
            navigation.row: 19

            Dropdown {
                id: textStyles

                width: parent.width

                navigation.name: "Text style Value"
                navigation.panel: root.navigation
                navigation.row: 20

                model: [
                    { text: qsTrc("inspector", "Title"), value: TextTypes.TEXT_TYPE_TITLE },
                    { text: qsTrc("inspector", "Subtitle"), value: TextTypes.TEXT_TYPE_SUBTITLE},
                    { text: qsTrc("inspector", "Composer"), value: TextTypes.TEXT_TYPE_COMPOSER },
                    { text: qsTrc("inspector", "Lyricist"), value: TextTypes.TEXT_TYPE_LYRICS_ODD },
                    { text: qsTrc("inspector", "Translator"), value: TextTypes.TEXT_TYPE_TRANSLATOR },
                    { text: qsTrc("inspector", "Frame"), value: TextTypes.TEXT_TYPE_FRAME },
                    { text: qsTrc("inspector", "Header"), value: TextTypes.TEXT_TYPE_HEADER },
                    { text: qsTrc("inspector", "Footer"), value: TextTypes.TEXT_TYPE_FOOTER },
                    { text: qsTrc("inspector", "Measure number"), value: TextTypes.TEXT_TYPE_MEASURE_NUMBER },
                    { text: qsTrc("inspector", "Instrument name (Part)"), value: TextTypes.TEXT_TYPE_INSTRUMENT_EXCERPT },
                    { text: qsTrc("inspector", "Instrument change"), value: TextTypes.TEXT_TYPE_INSTRUMENT_CHANGE },
                    { text: qsTrc("inspector", "Staff"), value: TextTypes.TEXT_TYPE_STAFF },
                    { text: qsTrc("inspector", "System"), value: TextTypes.TEXT_TYPE_SYSTEM },
                    { text: qsTrc("inspector", "Expression"), value: TextTypes.TEXT_TYPE_EXPRESSION },
                    { text: qsTrc("inspector", "Dynamics"), value: TextTypes.TEXT_TYPE_DYNAMICS },
                    { text: qsTrc("inspector", "Hairpin"), value: TextTypes.TEXT_TYPE_HAIRPIN },
                    { text: qsTrc("inspector", "Tempo"), value: TextTypes.TEXT_TYPE_TEMPO },
                    { text: qsTrc("inspector", "Rehearshal mark"), value: TextTypes.TEXT_TYPE_REHEARSAL_MARK },
                    { text: qsTrc("inspector", "Repeat text left"), value: TextTypes.TEXT_TYPE_REPEAT_LEFT },
                    { text: qsTrc("inspector", "Repeat text right"), value: TextTypes.TEXT_TYPE_REPEAT_RIGHT },
                    { text: qsTrc("inspector", "Lyrics odd lines"), value: TextTypes.TEXT_TYPE_LYRICS_ODD },
                    { text: qsTrc("inspector", "Lyrics even lines"), value: TextTypes.TEXT_TYPE_LYRICS_EVEN },
                    { text: qsTrc("inspector", "Chord symbol"), value: TextTypes.TEXT_TYPE_HARMONY_A },
                    { text: qsTrc("inspector", "Chord symbol (Alternate)"), value: TextTypes.TEXT_TYPE_HARMONY_B },
                    { text: qsTrc("inspector", "Roman numeral analysis"), value: TextTypes.TEXT_TYPE_HARMONY_ROMAN },
                    { text: qsTrc("inspector", "Nashville number"), value: TextTypes.TEXT_TYPE_HARMONY_NASHVILLE },
                    { text: qsTrc("inspector", "Sticking"), value: TextTypes.TEXT_TYPE_STICKING }
                ]

                currentIndex: root.model && !root.model.textType.isUndefined ? textStyles.indexOfValue(root.model.textType.value) : -1

                onCurrentValueChanged: {
                    root.model.textType.value = textStyles.currentValue
                }
            }
        }

        RadioButtonGroup {
            id: textPositionButtonList

            height: 30
            width: parent.width

            model: [
                { textRole: qsTrc("inspector", "Above"), valueRole: TextTypes.TEXT_PLACEMENT_ABOVE },
                { textRole: qsTrc("inspector", "Below"), valueRole: TextTypes.TEXT_PLACEMENT_BELOW }
            ]

            delegate: FlatRadioButton {

                navigation.name: "Position" + model.index
                navigation.panel: root.navigation
                navigation.row: 21 + model.index

                ButtonGroup.group: textPositionButtonList.radioButtonGroup

                checked: root.model && !root.model.textPlacement.isUndefined ? root.model.textPlacement.value === modelData["valueRole"]
                                                                             : false
                onToggled: {
                    root.model.textPlacement.value = modelData["valueRole"]
                }

                StyledTextLabel {
                    text: modelData["textRole"]

                    elide: Text.ElideRight
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter
                }
            }
        }

        FlatButton {
            width: parent.width

            navigation.name: "Staff text properties"
            navigation.panel: root.navigation
            navigation.row: 24

            text: qsTrc("inspector", "Staff text properties")

            visible: root.model ? root.model.areStaffTextPropertiesAvailable : false

            onClicked: {
                root.model.showStaffTextProperties()
            }
        }
    }
}
