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

    property NavigationSection notationViewNavigationSection: null
    property QtObject model: textStyleModel
    property int navigationOrderStart: 0
    property int navigationOrderEnd: 0

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

        ColumnLayout {
            StyledDropdown {
                id: fontDropdown

                Layout.fillWidth: true
                Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft

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

                currentIndex: textStyleModel.textSettingsModel.fontFamily
                                && !textStyleModel.textSettingsModel.fontFamily.isUndefined
                                ? indexOfValue(textStyleModel.textSettingsModel.fontFamily.value)
                                : -1

                onActivated: function(index, value) {
                    textStyleModel.textSettingsModel.fontFamily.value = value
                }
            }

            RowLayout {
                RadioButtonGroup {
                    id: textStyleButtonGroup

                    height: 30
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
                        toolTipTitle: modelData.title

                        icon: modelData.iconCode

                        checked: !textStyleModel.textSettingsModel.fontStyle.value.isUndefined
                                    && (textStyleModel.textSettingsModel.fontStyle.value & modelData.value)

                        onToggled: {
                            textStyleModel.textSettingsModel.fontStyle.value = checked
                                ? textStyleModel.textSettingsModel.fontStyle.value & ~modelData.value
                                : textStyleModel.textSettingsModel.fontStyle.value | modelData.value
                        }
                    }
                }

                RadioButtonGroup {
                    id: horizontalAlignmentButtonList

                    height: 30
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
                IncrementalPropertyControl {
                    id: fontSizeSpinBox

                    Layout.preferredWidth: 60

                    currentValue: textStyleModel.textSettingsModel
                                    ? textStyleModel.textSettingsModel.fontSize.value
                                    : null

                    measureUnitsSymbol: qsTrc("global", "pt")

                    decimals: 1
                    step: 1
                    minValue: 1
                    maxValue: 99

                    onValueEditingFinished: function(newValue) {
                        if (textStyleModel.textSettingsModel) {
                            textStyleModel.textSettingsModel.fontSize.value = newValue
                        }
                    }
                }

                RadioButtonGroup {
                    id: verticalAlignmentButtonList

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
                RadioButtonGroup {
                    id: subscriptOptionsButtonList

                    height: 30

                    model: [
                        { iconRole: IconCode.TEXT_SUBSCRIPT, typeRole: TextTypes.TEXT_SUBSCRIPT_BOTTOM, titleRole: qsTrc("inspector", "Subscript") },
                        { iconRole: IconCode.TEXT_SUPERSCRIPT, typeRole: TextTypes.TEXT_SUBSCRIPT_TOP, titleRole: qsTrc("inspector", "Superscript") }
                    ]

                    delegate: FlatRadioButton {
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
                    text: qsTrc("inspector", "Line spacing:")
                }

                IncrementalPropertyControl {
                    id: lineSpacingSpinBox

                    Layout.preferredWidth: 60

                    currentValue: textStyleModel.textSettingsModel
                                    ? textStyleModel.textSettingsModel.textLineSpacing.value
                                    : null

                    measureUnitsSymbol: qsTrc("global", "li")

                    decimals: 1
                    step: 1
                    minValue: 1
                    maxValue: 99

                    onValueEditingFinished: function(newValue) {
                        if (textStyleModel.textSettingsModel) {
                            textStyleModel.textSettingsModel.textLineSpacing.value = newValue
                        }
                    }
                }
            }
        }

        RowLayout {
            FlatButton {
                Layout.preferredWidth: 90
                Layout.fillHeight: true

                icon: IconCode.FLAT
                text: qsTrc("inspector", "Add symbols")

                visible: textStyleModel.textSettingsModel.isSpecialCharactersInsertionAvailable

                onClicked: {
                    if (textStyleModel.textSettingsModel) {
                        textStyleModel.textSettingsModel.insertSpecialCharacters()
                    }
                }
            }

            ColumnLayout {
                Layout.preferredWidth: 90

                FlatButton {
                    Layout.fillWidth: true

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
                    Layout.fillWidth: true

                    visible: !root.textSettingsModel.isDynamicSpecificSettings

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
