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
import QtQuick 2.9
import QtQuick.Layouts 1.0
import QtQuick.Controls 2.0
import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0
import MuseScore.Inspector 1.0
import "../common"
import "../general/playback"

InspectorSectionView {
    id: root

    implicitHeight: contentColumn.height
    contentHeight: implicitHeight

    Column {
        id: contentColumn

        height: implicitHeight
        width: parent.width

        spacing: 16

        InspectorPropertyView {
            titleText: qsTrc("inspector", "Font")

            navigation.panel: root.navigationPanel
            navigation.name: "FontMenu"
            navigation.row: root.navigationRow(1)

            propertyItem: root.model ? root.model.fontFamily : null

            Dropdown {
                id: fontFamilyComboBox

                navigation.panel: root.navigationPanel
                navigation.name: "FontFamily"
                navigation.row: root.navigationRow(2)

                width: parent.width

                textRole: "text"
                valueRole: "text"

                model: {
                    var resultList = []

                    var fontFamilies = Qt.fontFamilies()

                    for (var i = 0; i < fontFamilies.length; ++i) {
                        resultList.push({"text" : fontFamilies[i]})
                    }

                    return resultList
                }

                currentIndex: root.model && !root.model.fontFamily.isUndefined ? fontFamilyComboBox.indexOfValue(root.model.fontFamily.value) : -1

                onCurrentValueChanged: {
                    root.model.fontFamily.value = fontFamilyComboBox.currentValue
                }
            }
        }

        Item {
            height: childrenRect.height
            width: parent.width

            InspectorPropertyView {
                anchors.left: parent.left
                anchors.right: parent.horizontalCenter
                anchors.rightMargin: 2

                navigation.panel: root.navigationPanel
                navigation.name: "StyleMenu"
                navigation.row: root.navigationRow(3)

                titleText: qsTrc("inspector", "Style")
                propertyItem: root.model ? root.model.fontStyle : null

                RadioButtonGroup {
                    height: 30
                    width: implicitWidth

                    model: [
                        { iconRole: IconCode.TEXT_BOLD, valueRole: TextTypes.FONT_STYLE_BOLD },
                        { iconRole: IconCode.TEXT_ITALIC, valueRole: TextTypes.FONT_STYLE_ITALIC  },
                        { iconRole: IconCode.TEXT_UNDERLINE, valueRole: TextTypes.FONT_STYLE_UNDERLINE  }
                    ]

                    delegate: FlatToggleButton {

                        navigation.panel: root.navigationPanel
                        navigation.name: "FontStyle"+model.index
                        navigation.row: root.navigationRow(model.index + 4)

                        icon: modelData["iconRole"]

                        checked: root.model && !root.model.fontStyle.isUndefined ? root.model.fontStyle.value & modelData["valueRole"] : false

                        backgroundColor: ui.theme.backgroundPrimaryColor

                        onToggled: {
                            root.model.fontStyle.value = checked ? root.model.fontStyle.value & ~modelData["valueRole"]
                                                                 : root.model.fontStyle.value | modelData["valueRole"]
                        }
                    }
                }
            }

            InspectorPropertyView {
                anchors.left: parent.horizontalCenter
                anchors.leftMargin: 2
                anchors.right: parent.right

                navigation.panel: root.navigationPanel
                navigation.name: "SizeMenu"
                navigation.row: root.navigationRow(7)

                titleText: qsTrc("inspector", "Size")
                propertyItem: root.model ? root.model.fontSize : null

                Dropdown {
                    id: sizes

                    width: parent.width

                    navigation.panel: root.navigationPanel
                    navigation.name: "Size"
                    navigation.row: root.navigationRow(8)

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

                    currentIndex: root.model && !root.model.fontSize.isUndefined ? sizes.indexOfValue(root.model.fontSize.value) : -1

                    onCurrentValueChanged: {
                        root.model.fontSize.value = sizes.currentValue
                    }
                }
            }
        }

        InspectorPropertyView {
            titleText: qsTrc("inspector", "Alignment")
            propertyItem: root.model ? root.model.horizontalAlignment : null

            navigation.panel: root.navigationPanel
            navigation.name: "AlignmentMenu"
            navigation.row: root.navigationRow(9)

            Item {
                height: childrenRect.height
                width: parent.width

                RadioButtonGroup {
                    id: horizontalAlignmentButtonList

                    anchors.left: parent.left
                    anchors.right: parent.horizontalCenter
                    anchors.rightMargin: 2

                    height: 30

                    model: [
                        { iconRole: IconCode.TEXT_ALIGN_LEFT, typeRole: TextTypes.FONT_ALIGN_H_LEFT },
                        { iconRole: IconCode.TEXT_ALIGN_CENTER, typeRole: TextTypes.FONT_ALIGN_H_CENTER },
                        { iconRole: IconCode.TEXT_ALIGN_RIGHT, typeRole: TextTypes.FONT_ALIGN_H_RIGHT }
                    ]

                    delegate: FlatRadioButton {

                        navigation.panel: root.navigationPanel
                        navigation.name: "HAlign"+model.index
                        navigation.row: root.navigationRow(model.index + 10)

                        width: 30

                        ButtonGroup.group: horizontalAlignmentButtonList.radioButtonGroup

                        checked: root.model && !root.model.horizontalAlignment.isUndefined ? root.model.horizontalAlignment.value === modelData["typeRole"]
                                                                                           : false

                        normalStateColor: ui.theme.backgroundPrimaryColor

                        onToggled: {
                            root.model.horizontalAlignment.value = modelData["typeRole"]
                        }

                        StyledIconLabel {
                            iconCode: modelData["iconRole"]
                        }
                    }
                }

                RadioButtonGroup {
                    id: verticalAlignmentButtonList

                    anchors.left: parent.horizontalCenter
                    anchors.leftMargin: 2
                    anchors.right: parent.right

                    height: 30

                    model: [
                        { iconRole: IconCode.TEXT_ALIGN_UNDER, typeRole: TextTypes.FONT_ALIGN_V_BOTTOM },
                        { iconRole: IconCode.TEXT_ALIGN_MIDDLE, typeRole: TextTypes.FONT_ALIGN_V_CENTER },
                        { iconRole: IconCode.TEXT_ALIGN_BASELINE, typeRole: TextTypes.FONT_ALIGN_V_BASELINE },
                        { iconRole: IconCode.TEXT_ALIGN_ABOVE, typeRole: TextTypes.FONT_ALIGN_V_TOP }
                    ]

                    delegate: FlatRadioButton {

                        navigation.panel: root.navigationPanel
                        navigation.name: "VAlign"+model.index
                        navigation.row: root.navigationRow(model.index + 13)

                        width: 30

                        ButtonGroup.group: verticalAlignmentButtonList.radioButtonGroup

                        checked: root.model && !root.model.verticalAlignment.isUndefined ? root.model.verticalAlignment.value === modelData["typeRole"]
                                                                                         : false

                        normalStateColor: ui.theme.backgroundPrimaryColor

                        onToggled: {
                            root.model.verticalAlignment.value = modelData["typeRole"]
                        }

                        StyledIconLabel {
                            iconCode: modelData["iconRole"]
                        }
                    }
                }
            }
        }

        FlatButton {
            width: parent.width

            navigation.panel: root.navigationPanel
            navigation.name: "Insert special charachters"
            navigation.row: root.navigationRow(18)

            text: qsTrc("inspector", "Insert special charachters")

            visible: root.model ? root.model.isSpecialCharactersInsertionAvailable : false

            onClicked: {
                if (root.model) {
                    root.model.insertSpecialCharacters()
                }
            }
        }

        TextAdvancedSettings {
            id: textAdvancedSettingsButton

            navigation.panel: root.navigationPanel
            navigation.name: "TextAdvancedSettings"
            navigation.row: root.navigationRow(19)

            width: contentColumn.width
            model: root.model
        }
    }
}
