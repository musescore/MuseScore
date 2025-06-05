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

import Muse.UiComponents 1.0
import Muse.Ui 1.0
import MuseScore.Inspector 1.0

import "../common"

InspectorSectionView {
    id: root

    implicitHeight: contentColumn.height

    Column {
        id: contentColumn

        height: implicitHeight
        width: parent.width

        spacing: 12

        DropdownPropertyView {
            id: fontSection
            navigationName: "Font"
            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigationRowStart + 1

            titleText: qsTrc("inspector", "Font")
            propertyItem: root.model ? root.model.fontFamily : null

            dropdown.textRole: "text"
            dropdown.valueRole: "text"

            model: {
                var resultList = []

                var fontFamilies = Qt.fontFamilies()

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
                        navigation.panel: root.navigationPanel
                        navigation.name: "FontStyle" + model.index
                        navigation.row: styleGroup.navigationRowStart + model.index
                        navigation.accessible.name: modelData.title

                        toolTipTitle: modelData.title

                        icon: modelData.iconCode

                        checked: root.model && !root.model.fontStyle.isUndefined && (root.model.fontStyle.value & modelData.value)

                        onToggled: {
                            root.model.fontStyle.value = checked ? root.model.fontStyle.value & ~modelData.value
                                                                 : root.model.fontStyle.value | modelData.value
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
                            iconRole: IconCode.TEXT_ALIGN_LEFT,
                            typeRole: TextTypes.FONT_ALIGN_H_LEFT,
                            title: qsTrc("inspector", "Align left"),
                            description: qsTrc("inspector", "Align left edge of text to reference point")
                        },
                        {
                            iconRole: IconCode.TEXT_ALIGN_CENTER,
                            typeRole: TextTypes.FONT_ALIGN_H_CENTER,
                            title: qsTrc("inspector", "Align center"),
                            description: qsTrc("inspector", "Align horizontal center of text to reference point")
                        },
                        {
                            iconRole: IconCode.TEXT_ALIGN_RIGHT,
                            typeRole: TextTypes.FONT_ALIGN_H_RIGHT,
                            title: qsTrc("inspector", "Align right"),
                            description: qsTrc("inspector", "Align right edge of text to reference point")
                        },
                        {
                            iconRole: IconCode.TEXT_ALIGN_JUSTIFY, 
                            typeRole: TextTypes.FONT_ALIGN_H_JUSTIFY,
                            title: qsTrc("inspector", "Justify"),
                            description: qsTrc("inspector", "Justify text to fill the available width")
                        }
                    ]

                    delegate: FlatRadioButton {
                        navigation.panel: root.navigationPanel
                        navigation.name: "HAlign" + model.index
                        navigation.row: horizontalAlignmentButtonList.navigationRowStart + model.index
                        navigation.accessible.name: modelData.title
                        navigation.accessible.description: modelData.description

                        toolTipTitle: modelData.title
                        toolTipDescription: modelData.description

                        width: 30
                        transparent: true

                        iconCode: modelData.iconRole
                        checked: root.model && !root.model.horizontalAlignment.isUndefined && (root.model.horizontalAlignment.value === modelData.typeRole)
                        onToggled: {
                            root.model.horizontalAlignment.value = modelData.typeRole
                        }
                    }
                }

                RadioButtonGroup {
                    id: verticalAlignmentButtonList

                    property int navigationRowStart: horizontalAlignmentButtonList.navigationRowEnd + 1
                    property int navigationRowEnd: navigationRowStart + count

                    anchors.left: parent.horizontalCenter
                    anchors.leftMargin: 2
                    anchors.right: parent.right

                    height: 30

                    model: [
                        {
                            iconRole: IconCode.TEXT_ALIGN_TOP,
                            typeRole: TextTypes.FONT_ALIGN_V_TOP,
                            title: qsTrc("inspector", "Align top"),
                            description: qsTrc("inspector", "Align top edge of text to reference point")
                        },
                        {
                            iconRole: IconCode.TEXT_ALIGN_MIDDLE,
                            typeRole: TextTypes.FONT_ALIGN_V_CENTER,
                            title: qsTrc("inspector", "Align middle"),
                            description: qsTrc("inspector", "Align vertical center of text to reference point")
                        },
                        {
                            iconRole: IconCode.TEXT_ALIGN_BOTTOM,
                            typeRole: TextTypes.FONT_ALIGN_V_BOTTOM,
                            title:qsTrc("inspector", "Align bottom"),
                            description: qsTrc("inspector", "Align bottom edge of text to reference point")
                        },
                        {
                            iconRole: IconCode.TEXT_ALIGN_BASELINE,
                            typeRole: TextTypes.FONT_ALIGN_V_BASELINE,
                            title: qsTrc("inspector", "Align baseline"),
                            description: qsTrc("inspector", "Align baseline of text to reference point")
                        }
                    ]

                    delegate: FlatRadioButton {
                        navigation.panel: root.navigationPanel
                        navigation.name: "VAlign" + model.index
                        navigation.row: verticalAlignmentButtonList.navigationRowStart + model.index
                        navigation.accessible.name: modelData.title
                        navigation.accessible.description: modelData.description

                        toolTipTitle: modelData.title
                        toolTipDescription: modelData.description

                        width: 30
                        transparent: true

                        iconCode: modelData.iconRole
                        checked: root.model && !root.model.verticalAlignment.isUndefined && (root.model.verticalAlignment.value === modelData.typeRole)
                        onToggled: {
                            root.model.verticalAlignment.value = modelData.typeRole
                        }
                    }
                }
            }
        }

        FlatButton {
            id: insertSpecCharactersButton
            width: parent.width

            navigation.panel: root.navigationPanel
            navigation.name: "Insert special characters"
            navigation.row: alignmentSection.navigationRowEnd + 1

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
