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

import MuseScore.NotationScene 1.0
import MuseScore.Inspector 1.0

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

    function updatePosition(pos, size) {
        var h = contentRow.height + 120
        root.x = pos.x + size.x / 4
        root.y = pos.y - h / 2
    }

    TextStylePopupModel {
        id: textStyleModel

        onItemRectChanged: function(rect) {
            updatePosition(Qt.point(rect.x, rect.y), Qt.point(rect.width, rect.height))
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

                model: {
                    var resultList = []

                    var fontFamilies = Qt.fontFamilies()

                    for (var i = 0; i < fontFamilies.length; ++i) {
                        resultList.push({"text" : fontFamilies[i]})
                    }

                    return resultList

                }

                currentIndex: 0 //indexOfValue(/*TODO*/))

                onActivated: function(index, value) {
                    //TODO
                }
            }

            RowLayout { //Column 1, Row 2
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

                        checked: false //currentTextStyle(/*TODO*/)

                        onToggled: {
                            //TODO
                        }
                    }
                }

                RadioButtonGroup {
                    id: horizontalAlignmentButtonList

                    height: 30
                    width: implicitWidth

                    enabled: true //currentHorizontalAlignment(/*TODO*/)

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

                        checked: false //currentTextHAlign(/*TODO*/)

                        onToggled: {
                            //TODO
                        }
                    }
                }
            }
        }
        ColumnLayout {
            RowLayout { //Column 2, Row 1
                IncrementalPropertyControl {
                    id: textStyleSpinBox

                    Layout.preferredWidth: 60

                    isIndeterminate: false //TODO
                    currentValue: 0 //TODO

                    onValueEditingFinished: function(newValue) {
                        //TODO
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

                        checked: false //currentVerticalAlignment(/*TODO*/)

                        onToggled: {
                            //TODO
                        }
                    }
                }
            }

            RowLayout { //Column 2, Row 2
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

                        transparent: true

                        iconCode: modelData["iconRole"]
                        checked: false //currentSubscriptOption(/*TODO*)
                        onClicked: {
                            //TODO
                        }
                    }
                }

                StyledTextLabel {
                    text: qsTrc("inspector", "Line spacing:")
                }

                IncrementalPropertyControl {
                    id: lineSpacingSpinBox

                    Layout.preferredWidth: 60

                    isIndeterminate: false //TODO
                    currentValue: 0 //TODO

                    onValueEditingFinished: function(newValue) {
                        //TODO
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

                onClicked: {
                    root.model.insertSpecialCharacters()
                }
            }

            ColumnLayout {
                Layout.preferredWidth: 90

                FlatButton {
                    Layout.fillWidth: true
                    text: qsTrc("inspector", "Text style")
                }

                FlatButton {
                    Layout.fillWidth: true
                    text: qsTrc("inspector", "Frame")
                }
            }
        }
    }
}
