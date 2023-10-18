/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore BVBA and others
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
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import Muse.Ui 1.0
import Muse.UiComponents 1.0

import MuseScore.Inspector 1.0

import "../../common"

StyledPopupView {
    id: root

    property QtObject model: textStyleModel

    //! Note: the navigation order does not match the order of the components in the file
    property NavigationSection notationViewNavigationSection: null
    property int navigationOrderStart: 0
    property int navigationOrderEnd: textStyleSettingsNavPanel.order

    contentWidth: contentRow.width
    contentHeight: contentRow.height

    margins: 10

    showArrow: false

    function updatePosition(elementRect) {
        root.x = elementRect.x + elementRect.width / 5
        root.y = elementRect.y - root.contentHeight - 30
    }

    TextStylePopupModel {
        id: textStyleModel

        onItemRectChanged: function(rect) {
            updatePosition(rect)
        }
    }

    Component.onCompleted: {
        textStyleModel.init()
    }

    RowLayout {
        id: contentRow

        NavigationPanel {
            id: textStyleSettingsNavPanel
            name: "TextStyleSettings"
            direction: NavigationPanel.Vertical
            section: root.notationViewNavigationSection
            order: root.navigationOrderStart
            accessible.name: qsTrc("inspector", "Text style settings buttons")
        }

        ColumnLayout {
            StyledDropdown { //1
                id: fontDropdown

                Layout.fillWidth: true
                Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft

                navigation.name: "fontDropdown"
                navigation.panel: textStyleSettingsNavPanel
                navigation.row: 1
                navigation.accessible.name: qsTrc("inspector", "Font dropdown")

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

                currentIndex:  !textStyleModel.textSettingsModel.fontFamily.isUndefined
                                ? indexOfValue(textStyleModel.textSettingsModel.fontFamily.value)
                                : -1

                onActivated: function(index, value) {
                    textStyleModel.textSettingsModel.fontFamily.value = value
                }
            }

            RowLayout { //6
                RadioButtonGroup {
                    id: textStyleButtonGroup

                    height: 30
                    width: implicitWidth

                    property int navigationRowStart: textStylePopupButton.navigation.row + 1
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
                        navigation.name: "textStyleButtonGroup"
                        navigation.panel: textStyleSettingsNavPanel
                        navigation.row: textStyleButtonGroup.navigationRowStart + model.index
                        navigation.accessible.name: qsTrc("inspector", "Text style buttons")

                        toolTipTitle: modelData.title

                        icon: modelData.iconCode

                        checked: !textStyleModel.textSettingsModel.fontStyle.isUndefined
                                    && (textStyleModel.textSettingsModel.fontStyle.value & modelData.value)

                        onToggled: {
                            textStyleModel.textSettingsModel.fontStyle.value = checked
                                ? textStyleModel.textSettingsModel.fontStyle.value & ~modelData.value
                                : textStyleModel.textSettingsModel.fontStyle.value | modelData.value
                        }
                    }
                }

                RadioButtonGroup { //7
                    id: horizontalAlignmentButtonList

                    height: 30
                    width: implicitWidth

                    property int navigationRowStart: textStyleButtonGroup.navigationRowEnd + 1
                    property int navigationRowEnd: navigationRowStart + count

                    enabled: textStyleModel.textSettingsModel.isHorizontalAlignmentAvailable

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
                        }
                    ]

                    delegate: FlatRadioButton {
                        navigation.name: "horizonalAlignmentButtonList"
                        navigation.panel: textStyleSettingsNavPanel
                        navigation.row: horizontalAlignmentButtonList.navigationRowStart + model.index
                        navigation.accessible.name: qsTrc("inspector", "Horizontal alignent buttons")

                        width: 30

                        toolTipTitle: modelData.title
                        toolTipDescription: modelData.description

                        transparent: true

                        iconCode: modelData.iconRole

                        checked: !textStyleModel.textSettingsModel.horizontalAlignment.isUndefined
                                    && (textStyleModel.textSettingsModel.horizontalAlignment.value === modelData.typeRole)

                        onToggled: {
                            textStyleModel.textSettingsModel.horizontalAlignment.value = modelData.typeRole
                        }
                    }
                }
            }
        }
        ColumnLayout {
            RowLayout {
                IncrementalPropertyControl { //2
                    id: fontSizeSpinBox

                    navigation.name: "fontSizeSpinBox"
                    navigation.panel: textStyleSettingsNavPanel
                    navigation.row: fontDropdown.navigation.row + 1
                    navigation.accessible.name: qsTrc("inspector", "Font size spinbox")

                    Layout.preferredWidth: 60

                    currentValue: textStyleModel.textSettingsModel.fontSize.value

                    measureUnitsSymbol: qsTrc("global", "pt")

                    decimals: 1
                    step: 1
                    minValue: 1
                    maxValue: 99

                    onValueEditingFinished: function(newValue) {
                        textStyleModel.textSettingsModel.fontSize.value = newValue
                    }
                }

                RadioButtonGroup { //3
                    id: verticalAlignmentButtonList

                    property int navigationRowStart: fontSizeSpinBox.navigation.row + 1
                    property int navigationRowEnd: navigationRowStart + count

                    height: 30
                    width: implicitWidth

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
                        navigation.name: "verticalAlignmentButtonList"
                        navigation.panel: textStyleSettingsNavPanel
                        navigation.row: verticalAlignmentButtonList.navigationRowStart + model.index
                        navigation.accessible.name: qsTrc("inspector", "Vertical alignment buttons")

                        width: 30

                        toolTipTitle: modelData.title
                        toolTipDescription: modelData.description

                        transparent: true

                        iconCode: modelData.iconRole

                        checked: !textStyleModel.textSettingsModel.verticalAlignment.isUndefined
                                    && (textStyleModel.textSettingsModel.verticalAlignment.value === modelData.typeRole)

                        onToggled: {
                            textStyleModel.textSettingsModel.verticalAlignment.value = modelData.typeRole
                        }
                    }
                }
            }

            RowLayout {
                RadioButtonGroup { //8
                    id: subscriptOptionsButtonList

                    height: 30

                    property int navigationRowStart: horizontalAlignmentButtonList.navigationRowEnd + 1
                    property int navigationRowEnd: navigationRowStart + count

                    model: [
                        { iconRole: IconCode.TEXT_SUBSCRIPT, typeRole: TextTypes.TEXT_SUBSCRIPT_BOTTOM, titleRole: qsTrc("inspector", "Subscript") },
                        { iconRole: IconCode.TEXT_SUPERSCRIPT, typeRole: TextTypes.TEXT_SUBSCRIPT_TOP, titleRole: qsTrc("inspector", "Superscript") }
                    ]

                    delegate: FlatRadioButton {
                        navigation.name: "subscriptOptionsButtonList"
                        navigation.panel: textStyleSettingsNavPanel
                        navigation.row: subscriptOptionsButtonList.navigationRowStart + model.index
                        navigation.accessible.name: qsTrc("inspector", "Subscript buttons")

                        width: 30

                        toolTipTitle: modelData.titleRole
                        iconCode: modelData["iconRole"]

                        transparent: true

                        checked: !textStyleModel.textSettingsModel.textScriptAlignment.isUndefined
                                    ? textStyleModel.textSettingsModel.textScriptAlignment.value === modelData["typeRole"]
                                    : false

                        onClicked: {
                            if (textStyleModel.textSettingsModel.textScriptAlignment.value === modelData["typeRole"]) {
                                textStyleModel.textSettingsModel.textScriptAlignment.value = TextTypes.TEXT_SUBSCRIPT_NORMAL
                            } else {
                                textStyleModel.textSettingsModel.textScriptAlignment.value = modelData["typeRole"]
                            }
                        }
                    }
                }

                StyledTextLabel {
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                    text: qsTrc("inspector", "Line spacing:")
                }

                IncrementalPropertyControl { //9
                    id: lineSpacingSpinBox

                    navigation.name: "lineSpacingSpinBox"
                    navigation.panel: textStyleSettingsNavPanel
                    navigation.row: subscriptOptionsButtonList.navigationRowEnd + 1
                    navigation.accessible.name: qsTrc("inspector", "Line spacing spinbox")

                    Layout.preferredWidth: 60

                    currentValue: textStyleModel.textSettingsModel.textLineSpacing.value

                    measureUnitsSymbol: qsTrc("global", "li")

                    decimals: 1
                    step: 1
                    minValue: 1
                    maxValue: 99

                    onValueEditingFinished: function(newValue) {
                        textStyleModel.textSettingsModel.textLineSpacing.value = newValue
                    }
                }
            }
        }

        RowLayout {
            FlatButton { //4
                id: addSymbolsButton

                Layout.preferredWidth: 90
                Layout.fillHeight: true

                navigation.name: "addSymbolsButton"
                navigation.panel: textStyleSettingsNavPanel
                navigation.row: verticalAlignmentButtonList.navigationRowEnd + 1
                navigation.accessible.name: qsTrc("inspector", "Add symbols button")

                icon: IconCode.FLAT
                text: qsTrc("inspector", "Add symbols")

                visible: textStyleModel.textSettingsModel.isSpecialCharactersInsertionAvailable

                onClicked: {
                    textStyleModel.textSettingsModel.insertSpecialCharacters()
                }
            }

            ColumnLayout {
                Layout.preferredWidth: 90

                FlatButton { //5
                    id: textStylePopupButton

                    Layout.fillWidth: true

                    navigation.name: "textStylePopupButton"
                    navigation.panel: textStyleSettingsNavPanel
                    navigation.row: addSymbolsButton.navigation.row + 1
                    navigation.accessible.name: qsTrc("inspector", "Text style options")

                    text: qsTrc("inspector", "Text style")

                    onClicked: {
                        textStyleSubPopup.toggleOpened()
                    }

                    TextStyleSubPopup {
                        id: textStyleSubPopup

                        textSettingsModel: textStyleModel.textSettingsModel
                    }
                }

                FlatButton {
                    id: framePopupButton

                    Layout.fillWidth: true

                    navigation.name: "framePopupButton"
                    navigation.panel: textStyleSettingsNavPanel
                    navigation.row: lineSpacingSpinBox.navigation.row + 1
                    navigation.accessible.name: qsTrc("inspector", "Frame settings")

                    visible: !textStyleModel.textSettingsModel.isDynamicSpecificSettings

                    text: qsTrc("inspector", "Frame")

                    onClicked: {
                        frameSubPopup.toggleOpened()
                    }

                    FrameSubPopup {
                       id: frameSubPopup

                       textSettingsModel: textStyleModel.textSettingsModel
                    }
                }
            }
        }
    }
}
