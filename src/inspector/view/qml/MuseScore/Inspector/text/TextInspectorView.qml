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

import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0
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
                        { iconCode: IconCode.TEXT_BOLD, value: TextTypes.FONT_STYLE_BOLD, title: qsTrc("inspector", "Bold") },
                        { iconCode: IconCode.TEXT_ITALIC, value: TextTypes.FONT_STYLE_ITALIC, title: qsTrc("inspector", "Italic") },
                        { iconCode: IconCode.TEXT_UNDERLINE, value: TextTypes.FONT_STYLE_UNDERLINE, title: qsTrc("inspector", "Underline") },
                        { iconCode: IconCode.TEXT_STRIKE, value: TextTypes.FONT_STYLE_STRIKE, title: qsTrc("inspector", "Strike-through") }
                    ]

                    delegate: FlatToggleButton {
                        navigation.panel: root.navigationPanel
                        navigation.name: "FontStyle" + model.index
                        navigation.row: styleGroup.navigationRowStart + model.index
                        navigation.accessible.name: styleSection.titleText + " " + modelData["title"]

                        icon: modelData["iconCode"]

                        checked: root.model && !root.model.fontStyle.isUndefined ? root.model.fontStyle.value & modelData["value"] : false

                        onToggled: {
                            root.model.fontStyle.value = checked ? root.model.fontStyle.value & ~modelData["value"]
                                                                 : root.model.fontStyle.value | modelData["value"]
                        }
                    }
                }
            }

            DropdownPropertyView {
                id: sizeSection
                anchors.left: parent.horizontalCenter
                anchors.leftMargin: 2
                anchors.right: parent.right

                navigationName: "Size"
                navigationPanel: root.navigationPanel
                navigationRowStart: styleSection.navigationRowEnd + 1

                titleText: qsTrc("inspector", "Size")
                propertyItem: root.model ? root.model.fontSize : null

                model: [
                    { text: "8",  value: 8 },
                    { text: "9",  value: 9 },
                    { text: "10", value: 10 },
                    { text: "11", value: 11 },
                    { text: "12", value: 12 },
                    { text: "14", value: 14 },
                    { text: "16", value: 16 },
                    { text: "18", value: 18 },
                    { text: "24", value: 24 },
                    { text: "30", value: 30 },
                    { text: "36", value: 36 },
                    { text: "48", value: 48 }
                ]
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
                    id: horizontalAlignmentButtonList

                    property int navigationRowStart: alignmentSection.navigationRowStart + 1
                    property int navigationRowEnd: navigationRowStart + count

                    anchors.left: parent.left
                    anchors.right: parent.horizontalCenter
                    anchors.rightMargin: 2

                    height: 30

                    model: [
                        { iconRole: IconCode.TEXT_ALIGN_LEFT, typeRole: TextTypes.FONT_ALIGN_H_LEFT, title: qsTrc("inspector", "Left") },
                        { iconRole: IconCode.TEXT_ALIGN_CENTER, typeRole: TextTypes.FONT_ALIGN_H_CENTER, title: qsTrc("inspector", "Center") },
                        { iconRole: IconCode.TEXT_ALIGN_RIGHT, typeRole: TextTypes.FONT_ALIGN_H_RIGHT, title: qsTrc("inspector", "Right") }
                    ]

                    delegate: FlatRadioButton {
                        navigation.panel: root.navigationPanel
                        navigation.name: "HAlign"+model.index
                        navigation.row: horizontalAlignmentButtonList.navigationRowStart + model.index
                        navigation.accessible.name: alignmentSection.titleText + " " + qsTrc("inspector", "Horizontal") + " " + modelData["title"]

                        width: 30
                        transparent: true

                        iconCode: modelData["iconRole"]
                        checked: root.model && !root.model.horizontalAlignment.isUndefined ? root.model.horizontalAlignment.value === modelData["typeRole"]
                                                                                           : false
                        onToggled: {
                            root.model.horizontalAlignment.value = modelData["typeRole"]
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
                        { iconRole: IconCode.TEXT_ALIGN_UNDER, typeRole: TextTypes.FONT_ALIGN_V_BOTTOM, title: qsTrc("inspector", "Bottom") },
                        { iconRole: IconCode.TEXT_ALIGN_MIDDLE, typeRole: TextTypes.FONT_ALIGN_V_CENTER, title: qsTrc("inspector", "Center") },
                        { iconRole: IconCode.TEXT_ALIGN_BASELINE, typeRole: TextTypes.FONT_ALIGN_V_BASELINE, title: qsTrc("inspector", "Baseline") },
                        { iconRole: IconCode.TEXT_ALIGN_ABOVE, typeRole: TextTypes.FONT_ALIGN_V_TOP, title: qsTrc("inspector", "Top") }
                    ]

                    delegate: FlatRadioButton {
                        navigation.panel: root.navigationPanel
                        navigation.name: "VAlign"+model.index
                        navigation.row: verticalAlignmentButtonList.navigationRowStart + model.index
                        navigation.accessible.name: alignmentSection.titleText + " " + qsTrc("inspector", "Vertical") + " " + modelData["title"]

                        width: 30
                        transparent: true

                        iconCode: modelData["iconRole"]
                        checked: root.model && !root.model.verticalAlignment.isUndefined ? root.model.verticalAlignment.value === modelData["typeRole"]
                                                                                         : false
                        onToggled: {
                            root.model.verticalAlignment.value = modelData["typeRole"]
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

        PopupViewButton {
            id: textAdvancedSettingsButton

            width: contentColumn.width
            anchorItem: root.anchorItem

            navigation.panel: root.navigationPanel
            navigation.name: "TextAdvancedSettings"
            navigation.row: insertSpecCharactersButton.navigation.row + 1

            text: qsTrc("inspector", "More…")
            visible: root.model ? !root.model.isEmpty : false

            popupContent: TextSettings {
                model: root.model

                navigationPanel: textAdvancedSettingsButton.popupNavigationPanel
            }

            onEnsureContentVisibleRequested: function(invisibleContentHeight) {
                root.ensureContentVisibleRequested(invisibleContentHeight)
            }

            onPopupOpened: {
                root.popupOpened(textAdvancedSettingsButton.popup)
            }
        }
    }
}
