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
pragma ComponentBehavior: Bound

import QtQuick

import Muse.UiComponents
import Muse.Ui
import MuseScore.Inspector

import "../common"

InspectorSectionView {
    id: root

    required property TextSettingsModel model

    implicitHeight: contentColumn.height

    Column {
        id: contentColumn

        height: implicitHeight
        width: parent.width

        spacing: 12

        enabled: root.model ? root.model.areTextPropertiesAvailable : false

        DropdownPropertyView {
            id: fontSection
            navigationName: "Font"
            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigationRowStart + 1

            titleText: qsTrc("inspector", "Font")
            propertyItem: root.model ? root.model.fontFamily : null

            dropdownComp: Component {
                FontDropdown { }
            }

            dropdownTextRole: "text"
            dropdownValueRole: "text"

            model: {
                var resultList = []

                var fontFamilies = ui.allTextFonts()

                for (var i = 0; i < fontFamilies.length; ++i) {
                    resultList.push({"text" : fontFamilies[i]})
                }

                return resultList
            }
        }

        Item {
            height: childrenRect.height
            width: parent.width

            InspectorPropertyView {
                id: styleSection
                titleText: qsTrc("inspector", "Style")
                propertyItem: root.model ? root.model.fontStyle : null

                anchors.left: parent.left
                anchors.right: parent.horizontalCenter
                anchors.rightMargin: 2

                navigationName: "StyleMenu"
                navigationPanel: root.navigationPanel
                navigationRowStart: fontSection.navigationRowEnd + 1
                navigationRowEnd: styleGroup.navigationRowEnd

                RadioButtonGroup {
                    id: styleGroup
                    height: 30
                    width: implicitWidth

                    property int navigationRowStart: styleSection.navigationRowStart + 1
                    property int navigationRowEnd: navigationRowStart + count

                    model: [
                        {
                            iconCode: IconCode.TEXT_BOLD,
                            value: TextTypes.FONT_STYLE_BOLD,
                            title: qsTrc("inspector", "Bold")
                        },
                        {
                            iconCode: IconCode.TEXT_ITALIC,
                            value: TextTypes.FONT_STYLE_ITALIC,
                            title: qsTrc("inspector", "Italic")
                        },
                        {
                            iconCode: IconCode.TEXT_UNDERLINE,
                            value: TextTypes.FONT_STYLE_UNDERLINE,
                            title: qsTrc("inspector", "Underline")
                        },
                        {
                            iconCode: IconCode.TEXT_STRIKE,
                            value: TextTypes.FONT_STYLE_STRIKE,
                            title: qsTrc("inspector", "Strikethrough")
                        }
                    ]

                    delegate: FlatToggleButton {
                        required property int iconCode
                        required property int value
                        required property string title
                        required property int index

                        navigation.panel: root.navigationPanel
                        navigation.name: "FontStyle" + index
                        navigation.row: styleGroup.navigationRowStart + index
                        navigation.accessible.name: title

                        toolTipTitle: title
                        icon: iconCode

                        checked: root.model && !root.model.fontStyle.isUndefined && (root.model.fontStyle.value & value)

                        onToggled: {
                            root.model.fontStyle.value = checked ? root.model.fontStyle.value & ~value
                                                                 : root.model.fontStyle.value | value
                        }
                    }
                }
            }

            SpinBoxPropertyView {
                id: sizeSection
                anchors.left: parent.horizontalCenter
                anchors.leftMargin: 2
                anchors.right: parent.right

                navigationName: "Size"
                navigationPanel: root.navigationPanel
                navigationRowStart: styleSection.navigationRowEnd + 1

                titleText: qsTrc("inspector", "Size")
                measureUnitsSymbol: qsTrc("global", "pt")
                propertyItem: root.model ? root.model.fontSize : null

                decimals: 1
                step: 1
                minValue: 1
                maxValue: 99
            }
        }

        Column {
            height: implicitHeight
            width: parent.width

            spacing: 8

            InspectorPropertyView {
                id: alignmentSection
                titleText: qsTrc("inspector", "Alignment")
                propertyItem: root.model ? root.model.horizontalAlignment : null

                navigationName: "AlignmentMenu"
                navigationPanel: root.navigationPanel
                navigationRowStart: sizeSection.navigationRowEnd + 1
                navigationRowEnd: verticalAlignmentButtonList.navigationRowEnd

                isModified: root.model ? (root.model.horizontalAlignment.isModified
                                          || root.model.verticalAlignment.isModified) : false

                onRequestResetToDefault: {
                    if (root.model) {
                        root.model.horizontalAlignment.resetToDefault()
                        root.model.verticalAlignment.resetToDefault()
                    }
                }

                onRequestApplyToStyle: {
                    if (root.model) {
                        root.model.horizontalAlignment.applyToStyle()
                        root.model.verticalAlignment.applyToStyle()
                    }
                }

                Item {
                    height: childrenRect.height
                    width: parent.width

                    RadioButtonGroup {
                        enabled: root.model ? root.model.isHorizontalAlignmentAvailable : false
                        id: horizontalAlignmentButtonList

                        property int navigationRowStart: alignmentSection.navigationRowStart + 1
                        property int navigationRowEnd: navigationRowStart + count

                        anchors.left: parent.left
                        anchors.right: parent.horizontalCenter
                        anchors.rightMargin: 2

                        height: 30

                        model: [
                            {
                                iconCode: IconCode.TEXT_ALIGN_LEFT,
                                value: TextTypes.FONT_ALIGN_H_LEFT,
                                title: qsTrc("inspector", "Align left"),
                                description: qsTrc("inspector", "Left-align text within its bounding box")
                            },
                            {
                                iconCode: IconCode.TEXT_ALIGN_CENTER,
                                value: TextTypes.FONT_ALIGN_H_CENTER,
                                title: qsTrc("inspector", "Align center"),
                                description: qsTrc("inspector", "Horizontally center text within its bounding box")
                            },
                            {
                                iconCode: IconCode.TEXT_ALIGN_RIGHT,
                                value: TextTypes.FONT_ALIGN_H_RIGHT,
                                title: qsTrc("inspector", "Align right"),
                                description: qsTrc("inspector", "Right-align text within its bounding box")
                            },
                            {
                                iconCode: IconCode.TEXT_ALIGN_JUSTIFY,
                                value: TextTypes.FONT_ALIGN_H_JUSTIFY,
                                title: qsTrc("inspector", "Justify"),
                                description: qsTrc("inspector", "Justify text to fill the available width")
                            }
                        ]

                        delegate: FlatRadioButton {
                            required iconCode
                            required property int value
                            required property string title
                            required property string description
                            required property int index

                            navigation.panel: root.navigationPanel
                            navigation.name: "HAlign" + index
                            navigation.row: horizontalAlignmentButtonList.navigationRowStart + index
                            navigation.accessible.name: title
                            navigation.accessible.description: description

                            toolTipTitle: title
                            toolTipDescription: description

                            width: 30
                            transparent: true

                            checked: root.model && !root.model.horizontalAlignment.isUndefined && (root.model.horizontalAlignment.value === value)
                            onToggled: {
                                root.model.horizontalAlignment.value = value
                            }
                        }
                    }

                    RadioButtonGroup {
                        enabled: root.model ? root.model.isHorizontalAlignmentAvailable : false
                        id: positionButtonList

                        property int navigationRowStart: horizontalAlignmentButtonList.navigationRowEnd + 1
                        property int navigationRowEnd: navigationRowStart + count

                        anchors.leftMargin: 2
                        anchors.right: parent.right

                        height: 30

                        model: [
                            {
                                iconCode: root.model && root.model.usePositionRelativeToLine ? IconCode.ALIGN_LEFT : IconCode.NOTE_ALIGN_LEFT,
                                value: CommonTypes.LEFT,
                                title: root.model ? root.model.leftPositionText : "",
                                description: ""
                            },
                            {
                                iconCode: root.model && root.model.usePositionRelativeToLine ? IconCode.ALIGN_HORIZONTAL_CENTER : IconCode.NOTE_ALIGN_CENTER,
                                value:
                                CommonTypes.HCENTER,
                                title: root.model ? root.model.centerPositionText : "",
                                description: ""
                            },
                            {
                                iconCode: root.model && root.model.usePositionRelativeToLine ? IconCode.ALIGN_RIGHT : IconCode.NOTE_ALIGN_RIGHT,
                                value: CommonTypes.RIGHT,
                                title: root.model ? root.model.rightPositionText : "",
                                description: ""
                            }
                        ]

                        delegate: FlatRadioButton {
                            required iconCode
                            required property int value
                            required property string title
                            required property string description
                            required property int index

                            navigation.panel: root.navigationPanel
                            navigation.name: "HAlign" + index
                            navigation.row: positionButtonList.navigationRowStart + index
                            navigation.accessible.name: title
                            navigation.accessible.description: description

                            toolTipTitle: title
                            toolTipDescription: description

                            width: 30
                            transparent: true

                            checked: root.model && !root.model.horizontalPosition.isUndefined && (root.model.horizontalPosition.value === value)
                            onToggled: {
                                root.model.horizontalPosition.value = value
                            }
                        }
                    }
                }
            }

            RadioButtonGroup {
                id: verticalAlignmentButtonList

                property int navigationRowStart: positionButtonList.navigationRowEnd + 1
                property int navigationRowEnd: navigationRowStart + count

                anchors.left: parent.left
                anchors.right: parent.horizontalCenter
                anchors.rightMargin: 2

                height: 30

                model: [
                    {
                        iconCode: IconCode.TEXT_ALIGN_TOP,
                        value: TextTypes.FONT_ALIGN_V_TOP,
                        title: qsTrc("inspector", "Align top"),
                        description: qsTrc("inspector", "Align top edge of text to reference point")
                    },
                    {
                        iconCode: IconCode.TEXT_ALIGN_MIDDLE,
                        value: TextTypes.FONT_ALIGN_V_CENTER,
                        title: qsTrc("inspector", "Align middle"),
                        description: qsTrc("inspector", "Align vertical center of text to reference point")
                    },
                    {
                        iconCode: IconCode.TEXT_ALIGN_BOTTOM,
                        value: TextTypes.FONT_ALIGN_V_BOTTOM,
                        title:qsTrc("inspector", "Align bottom"),
                        description: qsTrc("inspector", "Align bottom edge of text to reference point")
                    },
                    {
                        iconCode: IconCode.TEXT_ALIGN_BASELINE,
                        value: TextTypes.FONT_ALIGN_V_BASELINE,
                        title: qsTrc("inspector", "Align baseline"),
                        description: qsTrc("inspector", "Align baseline of text to reference point")
                    }
                ]

                delegate: FlatRadioButton {
                    required iconCode
                    required property int value
                    required property string title
                    required property string description
                    required property int index

                    navigation.panel: root.navigationPanel
                    navigation.name: "VAlign" + index
                    navigation.row: verticalAlignmentButtonList.navigationRowStart + index
                    navigation.accessible.name: title
                    navigation.accessible.description: description

                    toolTipTitle: title
                    toolTipDescription: description

                    width: 30
                    transparent: true

                    checked: root.model && !root.model.verticalAlignment.isUndefined && (root.model.verticalAlignment.value === value)
                    onToggled: {
                        root.model.verticalAlignment.value = value
                    }
                }
            }
        }

        FlatButton {
            id: insertSpecCharactersButton
            width: parent.width

            navigation.panel: root.navigationPanel
            navigation.name: "Insert special characters"
            navigation.row: verticalAlignmentButtonList.navigationRowEnd + 1

            text: qsTrc("inspector", "Insert special characters")

            visible: root.model ? root.model.isSpecialCharactersInsertionAvailable : false

            onClicked: {
                if (root.model) {
                    root.model.insertSpecialCharacters()
                }
            }
        }

        ExpandableBlank {
            id: showItem
            isExpanded: false

            title: isExpanded ? qsTrc("inspector", "Show less") : qsTrc("inspector", "Show more")

            visible: root.model ? !root.model.isEmpty : false
            width: parent.width

            navigation.panel: root.navigationPanel
            navigation.name: "TextAdvancedSettings"
            navigation.row: insertSpecCharactersButton.navigation.row + 1

            contentItemComponent: TextSettings {
                model: root.model
                navigationPanel: root.navigationPanel
                navigationRowStart: showItem.navigation.row + 1
            }
        }
    }
}
