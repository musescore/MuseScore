/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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
import QtQuick
import QtQuick.Layouts

import Muse.Ui
import Muse.UiComponents

import MuseScore.Inspector
import MuseScore.NotationScene

import "../../common"

ColumnLayout {
    id:root
    required property var settingsModel
    property bool compact:false
    readonly property int elementSize:30
    readonly property int elementSpacing:4
    readonly property int controlSpacing:12
    readonly property int rowSpacing:8
    property NavigationPanel navigationPanel:null
    readonly property int verticalNavigationEnd:verticalAlignmentButtonList.navigationRowEnd
    readonly property int lineNavigationRow:lineSpacingSpinBox.navigation.row
    property int styleNavigationStart:1
    spacing: root.rowSpacing

    RowLayout {
        spacing: root.controlSpacing

        StyledDropdown { // 1
            id: fontDropdown

            Layout.fillWidth: true

            navigation.name: "fontDropdown"
            navigation.panel: root.navigationPanel
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

            currentIndex: !root.settingsModel.fontFamily.isUndefined
                          ? indexOfValue(root.settingsModel.fontFamily.value)
                          : -1

            onActivated: function (index, value) {
                root.settingsModel.fontFamily.value = value;
            }
        }

        IncrementalPropertyControl { // 2
            id: fontSizeSpinBox

            Layout.preferredWidth: root.compact ? 96 : 2 * root.elementSize + root.elementSpacing

            navigation.name: "fontSizeSpinBox"
            navigation.panel: root.navigationPanel
            navigation.row: fontDropdown.navigation.row + 1
            navigation.accessible.name: qsTrc("inspector", "Font size")

            isIndeterminate: root.settingsModel.fontSize.isUndefined
            currentValue: root.settingsModel.fontSize.value

            measureUnitsSymbol: qsTrc("global", "pt")

            decimals: 1
            step: 1
            minValue: 1
            maxValue: root.compact ? 1000 : 99

            onValueEditingFinished: function (newValue) {
                root.settingsModel.fontSize.value = newValue;
            }
        }

        RadioButtonGroup { // 3
            id: verticalAlignmentButtonList
            visible:!root.compact

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
                navigation.panel: root.navigationPanel
                navigation.row: verticalAlignmentButtonList.navigationRowStart + model.index
                navigation.accessible.name: qsTrc("inspector", "Vertical alignment buttons")

                toolTipTitle: modelData.title
                toolTipDescription: modelData.description

                transparent: true

                iconCode: modelData.iconRole

                checked: !root.settingsModel.verticalAlignment.isUndefined
                         && (root.settingsModel.verticalAlignment.value === modelData.typeRole)

                onToggled: {
                    root.settingsModel.verticalAlignment.value = modelData.typeRole;
                }
            }
        }
    }

    RowLayout {
        spacing: root.controlSpacing

        RadioButtonGroup { // 6
            id: textStyleButtonGroup

            property int navigationRowStart: root.styleNavigationStart
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
                navigation.panel: root.navigationPanel
                navigation.row: textStyleButtonGroup.navigationRowStart + model.index
                navigation.accessible.name: qsTrc("inspector", "Text style buttons")

                width: root.elementSize

                toolTipTitle: modelData.title

                icon: modelData.iconCode

                checked: !root.settingsModel.fontStyle.isUndefined
                         && (root.settingsModel.fontStyle.value & modelData.value)

                onToggled: {
                    root.settingsModel.fontStyle.value =
                        checked ? root.settingsModel.fontStyle.value & ~modelData.value
                                : root.settingsModel.fontStyle.value | modelData.value;
                }
            }
        }

        RadioButtonGroup { // 7
            id: horizontalAlignmentButtonList

            property int navigationRowStart: textStyleButtonGroup.navigationRowEnd + 1
            property int navigationRowEnd: navigationRowStart + count

            height: root.elementSize
            width: implicitWidth

            enabled: root.settingsModel.isHorizontalAlignmentAvailable

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
                navigation.name: "horizonalAlignmentButtonList"
                navigation.panel: root.navigationPanel
                navigation.row: horizontalAlignmentButtonList.navigationRowStart + model.index
                navigation.accessible.name: qsTrc("inspector", "Horizontal alignment buttons")

                width: root.elementSize

                toolTipTitle: modelData.title
                toolTipDescription: modelData.description

                transparent: true

                iconCode: modelData.iconRole

                checked: !root.settingsModel.horizontalAlignment.isUndefined
                         && (root.settingsModel.horizontalAlignment.value === modelData.typeRole)

                onToggled: {
                    root.settingsModel.horizontalAlignment.value = modelData.typeRole;
                }
            }
        }

        RadioButtonGroup { // 8
            id: subscriptOptionsButtonList
            visible:!root.compact

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
                navigation.panel: root.navigationPanel
                navigation.row: subscriptOptionsButtonList.navigationRowStart + model.index
                navigation.accessible.name: qsTrc("inspector", "Subscript buttons")

                width: root.elementSize

                toolTipTitle: modelData.titleRole
                iconCode: modelData["iconRole"]

                transparent: true

                checked: !root.settingsModel.textScriptAlignment.isUndefined
                         ? root.settingsModel.textScriptAlignment.value === modelData["typeRole"]
                         : false

                onClicked: {
                    if (root.settingsModel.textScriptAlignment.value === modelData["typeRole"]) {
                        root.settingsModel.textScriptAlignment.value = TextTypes.TEXT_SUBSCRIPT_NORMAL;
                    } else {
                        root.settingsModel.textScriptAlignment.value = modelData["typeRole"];
                    }
                }
            }
        }

        RowLayout {
            // 9
            visible:!root.compact
            spacing: root.elementSpacing

            StyledTextLabel {
                width: 2 * root.elementSize + root.elementSpacing
                horizontalAlignment: Text.AlignLeft

                text: qsTrc("inspector", "Line spacing:")
            }

            IncrementalPropertyControl {
                id: lineSpacingSpinBox

                navigation.name: "lineSpacingSpinBox"
                navigation.panel: root.navigationPanel
                navigation.row: subscriptOptionsButtonList.navigationRowEnd + 1
                navigation.accessible.name: qsTrc("inspector", "Line spacing")

                Layout.preferredWidth: root.compact ? 96 : 2 * root.elementSize + root.elementSpacing

                currentValue: root.settingsModel.textLineSpacing.value

                measureUnitsSymbol: qsTrc("global", "li")

                decimals: 2
                step: 0.1
                minValue: 0
                maxValue: 10

                onValueEditingFinished: function (newValue) {
                    root.settingsModel.textLineSpacing.value = newValue;
                }
            }
        }
    }
}
