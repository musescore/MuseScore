/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
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

    //! Note: the navigation order does not match the order of the components in the file
    property alias notationViewNavigationSection: textStyleSettingsNavPanel.section
    property alias navigationOrderStart: textStyleSettingsNavPanel.order
    readonly property alias navigationOrderEnd: textStyleSettingsNavPanel.order

    readonly property int elementSize: 30
    readonly property int elementSpacing: 4
    readonly property int controlSpacing: 12
    readonly property int rowSpacing: 8

    contentWidth: contentRow.width
    contentHeight: contentRow.height

    margins: root.controlSpacing

    showArrow: false

    focusPolicies: PopupView.DefaultFocus & ~PopupView.ClickFocus

    signal elementRectChanged(var elementRect)

    function updatePosition() {
        root.x = 0;
        root.y = -height;
    }

    TextStylePopupModel {
        id: textStyleModel

        onItemRectChanged: function (rect) {
            root.elementRectChanged(rect);
        }
    }

    Component.onCompleted: {
        textStyleModel.init();
    }

    RowLayout {
        id: contentRow
        spacing: root.controlSpacing

        NavigationPanel {
            id: textStyleSettingsNavPanel
            name: "TextStyleSettings"
            direction: NavigationPanel.Vertical
            accessible.name: qsTrc("inspector", "Text style settings buttons")
        }

        ColumnLayout {
            spacing: root.rowSpacing

            RowLayout {
                spacing: root.controlSpacing

                StyledDropdown { // 1
                    id: fontDropdown

                    Layout.fillWidth: true

                    navigation.name: "fontDropdown"
                    navigation.panel: textStyleSettingsNavPanel
                    navigation.row: 1
                    navigation.accessible.name: qsTrc("inspector", "Font")

                    textRole: "text"
                    valueRole: "text"

                    model: {
                        var resultList = [];

                        var fontFamilies = Qt.fontFamilies();

                        for (var i = 0; i < fontFamilies.length; ++i) {
                            resultList.push({
                                "text": fontFamilies[i]
                            });
                        }

                        return resultList;
                    }

                    currentIndex: !textStyleModel.textSettingsModel.fontFamily.isUndefined 
                                  ? indexOfValue(textStyleModel.textSettingsModel.fontFamily.value) 
                                  : -1

                    onActivated: function (index, value) {
                        textStyleModel.textSettingsModel.fontFamily.value = value;
                    }
                }

                IncrementalPropertyControl { // 2
                    id: fontSizeSpinBox

                    Layout.preferredWidth: 2 * root.elementSize + root.elementSpacing

                    navigation.name: "fontSizeSpinBox"
                    navigation.panel: textStyleSettingsNavPanel
                    navigation.row: fontDropdown.navigation.row + 1
                    navigation.accessible.name: qsTrc("inspector", "Font size")

                    isIndeterminate: textStyleModel.textSettingsModel.fontSize.isUndefined
                    currentValue: textStyleModel.textSettingsModel.fontSize.value

                    measureUnitsSymbol: qsTrc("global", "pt")

                    decimals: 1
                    step: 1
                    minValue: 1
                    maxValue: 99

                    onValueEditingFinished: function (newValue) {
                        textStyleModel.textSettingsModel.fontSize.value = newValue;
                    }
                }

                RadioButtonGroup { // 3
                    id: verticalAlignmentButtonList

                    property int navigationRowStart: fontSizeSpinBox.navigation.row + 1
                    property int navigationRowEnd: navigationRowStart + count

                    height: root.elementSize
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
                            title: qsTrc("inspector", "Align bottom"),
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
                        width: root.elementSize

                        navigation.name: "verticalAlignmentButtonList"
                        navigation.panel: textStyleSettingsNavPanel
                        navigation.row: verticalAlignmentButtonList.navigationRowStart + model.index
                        navigation.accessible.name: qsTrc("inspector", "Vertical alignment buttons")

                        toolTipTitle: modelData.title
                        toolTipDescription: modelData.description

                        transparent: true

                        iconCode: modelData.iconRole

                        checked: !textStyleModel.textSettingsModel.verticalAlignment.isUndefined 
                                 && (textStyleModel.textSettingsModel.verticalAlignment.value === modelData.typeRole)

                        onToggled: {
                            textStyleModel.textSettingsModel.verticalAlignment.value = modelData.typeRole;
                        }
                    }
                }
            }

            RowLayout {
                spacing: root.controlSpacing

                RadioButtonGroup { // 6
                    id: textStyleButtonGroup

                    property int navigationRowStart: textStylePopupButton.navigation.row + 1
                    property int navigationRowEnd: navigationRowStart + count

                    height: root.elementSize
                    width: implicitWidth

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

                        width: root.elementSize

                        toolTipTitle: modelData.title

                        icon: modelData.iconCode

                        checked: !textStyleModel.textSettingsModel.fontStyle.isUndefined 
                                 && (textStyleModel.textSettingsModel.fontStyle.value & modelData.value)

                        onToggled: {
                            textStyleModel.textSettingsModel.fontStyle.value = 
                                checked ? textStyleModel.textSettingsModel.fontStyle.value & ~modelData.value 
                                        : textStyleModel.textSettingsModel.fontStyle.value | modelData.value;
                        }
                    }
                }

                RadioButtonGroup { // 7
                    id: horizontalAlignmentButtonList

                    property int navigationRowStart: textStyleButtonGroup.navigationRowEnd + 1
                    property int navigationRowEnd: navigationRowStart + count

                    height: root.elementSize
                    width: implicitWidth

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
                        navigation.accessible.name: qsTrc("inspector", "Horizontal alignment buttons")

                        width: root.elementSize

                        toolTipTitle: modelData.title
                        toolTipDescription: modelData.description

                        transparent: true

                        iconCode: modelData.iconRole

                        checked: !textStyleModel.textSettingsModel.horizontalAlignment.isUndefined 
                                 && (textStyleModel.textSettingsModel.horizontalAlignment.value === modelData.typeRole)

                        onToggled: {
                            textStyleModel.textSettingsModel.horizontalAlignment.value = modelData.typeRole;
                        }
                    }
                }

                RadioButtonGroup { // 8
                    id: subscriptOptionsButtonList

                    height: root.elementSize

                    property int navigationRowStart: horizontalAlignmentButtonList.navigationRowEnd + 1
                    property int navigationRowEnd: navigationRowStart + count

                    model: [
                        {
                            iconRole: IconCode.TEXT_SUBSCRIPT,
                            typeRole: TextTypes.TEXT_SUBSCRIPT_BOTTOM,
                            titleRole: qsTrc("inspector", "Subscript")
                        },
                        {
                            iconRole: IconCode.TEXT_SUPERSCRIPT,
                            typeRole: TextTypes.TEXT_SUBSCRIPT_TOP,
                            titleRole: qsTrc("inspector", "Superscript")
                        }
                    ]

                    delegate: FlatRadioButton {
                        navigation.name: "subscriptOptionsButtonList"
                        navigation.panel: textStyleSettingsNavPanel
                        navigation.row: subscriptOptionsButtonList.navigationRowStart + model.index
                        navigation.accessible.name: qsTrc("inspector", "Subscript buttons")

                        width: root.elementSize

                        toolTipTitle: modelData.titleRole
                        iconCode: modelData["iconRole"]

                        transparent: true

                        checked: !textStyleModel.textSettingsModel.textScriptAlignment.isUndefined 
                                 ? textStyleModel.textSettingsModel.textScriptAlignment.value === modelData["typeRole"] 
                                 : false

                        onClicked: {
                            if (textStyleModel.textSettingsModel.textScriptAlignment.value === modelData["typeRole"]) {
                                textStyleModel.textSettingsModel.textScriptAlignment.value = TextTypes.TEXT_SUBSCRIPT_NORMAL;
                            } else {
                                textStyleModel.textSettingsModel.textScriptAlignment.value = modelData["typeRole"];
                            }
                        }
                    }
                }

                RowLayout {
                    // 9
                    spacing: root.elementSpacing

                    StyledTextLabel {
                        width: 2 * root.elementSize + root.elementSpacing
                        horizontalAlignment: Text.AlignLeft

                        text: qsTrc("inspector", "Line spacing:")
                    }

                    IncrementalPropertyControl {
                        id: lineSpacingSpinBox

                        navigation.name: "lineSpacingSpinBox"
                        navigation.panel: textStyleSettingsNavPanel
                        navigation.row: subscriptOptionsButtonList.navigationRowEnd + 1
                        navigation.accessible.name: qsTrc("inspector", "Line spacing")

                        Layout.preferredWidth: 2 * root.elementSize + root.elementSpacing

                        currentValue: textStyleModel.textSettingsModel.textLineSpacing.value

                        measureUnitsSymbol: qsTrc("global", "li")

                        decimals: 1
                        step: 1
                        minValue: 1
                        maxValue: 99

                        onValueEditingFinished: function (newValue) {
                            textStyleModel.textSettingsModel.textLineSpacing.value = newValue;
                        }
                    }
                }
            }
        }

        RowLayout {
            spacing: root.rowSpacing

            FlatButton { // 4
                id: addSymbolsButton

                Layout.preferredWidth: 3 * root.elementSize
                Layout.fillHeight: true

                navigation.name: "addSymbolsButton"
                navigation.panel: textStyleSettingsNavPanel
                navigation.row: verticalAlignmentButtonList.navigationRowEnd + 1

                icon: IconCode.FLAT
                text: qsTrc("inspector", "Add symbols")

                visible: textStyleModel.textSettingsModel.isSpecialCharactersInsertionAvailable

                onClicked: {
                    textStyleModel.textSettingsModel.insertSpecialCharacters();
                }
            }

            ColumnLayout {
                spacing: root.rowSpacing
                Layout.preferredWidth: 3 * root.elementSize

                FlatButton { // 5
                    id: textStylePopupButton

                    Layout.fillWidth: true

                    navigation.name: "textStylePopupButton"
                    navigation.panel: textStyleSettingsNavPanel
                    navigation.row: addSymbolsButton.navigation.row + 1

                    text: qsTrc("inspector", "Text style")

                    onClicked: {
                        textStyleSubPopup.toggleOpened();
                    }

                    TextStyleSubPopup {
                        id: textStyleSubPopup

                        textSettingsModel: textStyleModel.textSettingsModel
                    }
                }

                FlatButton { // 10
                    id: framePopupButton

                    Layout.fillWidth: true

                    navigation.name: "framePopupButton"
                    navigation.panel: textStyleSettingsNavPanel
                    navigation.row: lineSpacingSpinBox.navigation.row + 1

                    visible: !textStyleModel.textSettingsModel.isDynamicSpecificSettings

                    text: qsTrc("inspector", "Frame")

                    onClicked: {
                        frameSubPopup.toggleOpened();
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
