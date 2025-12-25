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
import QtQuick.Layouts

import MuseScore.NotationScene 1.0
import Muse.UiComponents
import Muse.Ui 1.0

StyledFlickable {
    id: root

    contentWidth: content.implicitWidth
    contentHeight: content.implicitHeight

    signal goToTextStylePage(string s)

    BendsPageModel {
        id: bendsModel
    }

    ColumnLayout {
        id: content
        spacing: 12

        StyledGroupBox {
            Layout.fillWidth: true
            Layout.minimumWidth: 500
            title: qsTrc("notation/editstyle/bends", "Bends")

            ColumnLayout {
                spacing: 8
                width: parent.width

                StyledGroupBox {
                    Layout.fillWidth: true
                    title: qsTrc("notation/editstyle/bends", "Standard staff")

                    StyleControlRowWithReset {
                        label: qsTrc("notation/editstyle/bends", "Line thickness:")
                        controlAreaWidth: 100
                        styleItem: bendsModel.guitarBendLineWidth
                        IncrementalPropertyControl {
                            measureUnitsSymbol: qsTrc("global", "sp")
                            step: 0.01
                            currentValue: bendsModel.guitarBendLineWidth.value
                            onValueEdited: function(newValue) {
                                bendsModel.guitarBendLineWidth.value = newValue
                            }
                        }
                    }
                }

                StyledGroupBox {
                    Layout.fillWidth: true
                    title: qsTrc("notation/editstyle/bends", "Tablature staff")

                    ColumnLayout {
                        spacing: 8
                        width: parent.width

                        StyleControlRowWithReset {
                            label: qsTrc("notation/editstyle/bends", "Line thickness:")
                            controlAreaWidth: 100
                            styleItem: bendsModel.guitarBendLineWidthTab
                            IncrementalPropertyControl {
                                measureUnitsSymbol: qsTrc("global", "sp")
                                step: 0.01
                                currentValue: bendsModel.guitarBendLineWidthTab.value
                                onValueEdited: function(newValue) {
                                    bendsModel.guitarBendLineWidthTab.value = newValue
                                }
                            }
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 12

                            StyleControlRowWithReset {
                                label: qsTrc("notation/editstyle/bends", "Arrow width:")
                                controlAreaWidth: 100
                                styleItem: bendsModel.guitarBendArrowWidth
                                IncrementalPropertyControl {
                                    measureUnitsSymbol: qsTrc("global", "sp")
                                    step: 0.1
                                    currentValue: bendsModel.guitarBendArrowWidth.value
                                    onValueEdited: function(newValue) {
                                        bendsModel.guitarBendArrowWidth.value = newValue
                                    }
                                }
                            }

                            StyleControlRowWithReset {
                                label: qsTrc("notation/editstyle/bends", "Arrow height:")
                                controlAreaWidth: 100
                                styleItem: bendsModel.guitarBendArrowHeight
                                IncrementalPropertyControl {
                                    measureUnitsSymbol: qsTrc("global", "sp")
                                    step: 0.1
                                    currentValue: bendsModel.guitarBendArrowHeight.value
                                    onValueEdited: function(newValue) {
                                        bendsModel.guitarBendArrowHeight.value = newValue
                                    }
                                }
                            }
                        }

                        StyleControlRowWithReset {
                            label: qsTrc("notation/editstyle/bends", "Label for full bends:")
                            styleItem: bendsModel.guitarBendUseFull
                            hasReset: false
                            RadioButtonGroup {
                                model: [
                                    { iconCode: IconCode.GUITAR_BEND_STYLE_1, text: "", value: false },
                                    { iconCode: IconCode.GUITAR_BEND_STYLE_FULL, text: "", value: true }
                                ]

                                delegate: FlatRadioButton {
                                    width: 106
                                    height: 60

                                    iconCode: modelData.iconCode
                                    iconFontSize: 28

                                    checked: modelData.value === bendsModel.guitarBendUseFull.value

                                    onToggled: {
                                        bendsModel.guitarBendUseFull.value = modelData.value
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        StyledGroupBox {
            Layout.fillWidth: true
            Layout.minimumWidth: 500
            title: qsTrc("notation/editstyle/bends", "Dives")

            ColumnLayout {
                spacing: 8
                width: parent.width

                StyledGroupBox {
                    Layout.fillWidth: true
                    title: qsTrc("notation/editstyle/bends", "Standard staff")

                    StyleControlRowWithReset {
                        label: qsTrc("notation/editstyle/bends", "Line thickness:")
                        controlAreaWidth: 100
                        styleItem: bendsModel.guitarDiveLineWidth
                        IncrementalPropertyControl {
                            measureUnitsSymbol: qsTrc("global", "sp")
                            step: 0.01
                            currentValue: bendsModel.guitarDiveLineWidth.value
                            onValueEdited: function(newValue) {
                                bendsModel.guitarDiveLineWidth.value = newValue
                            }
                        }
                    }
                }

                StyledGroupBox {
                    Layout.fillWidth: true
                    title: qsTrc("notation/editstyle/bends", "Tablature staff")

                    ColumnLayout {
                        spacing: 8
                        width: parent.width

                        StyledTextLabel {
                            horizontalAlignment: Text.AlignLeft
                            text: qsTrc("notation/editstyle/bends", "Show dive lines")
                        }

                        RadioButtonGroup {
                            orientation: Qt.Vertical
                            spacing: 8
                            model: [
                                { text: qsTrc("notation/editstyle/bends", "On the stave"), value: false },
                                { text: qsTrc("notation/editstyle/bends", "Above the stave"), value: true }
                            ]

                            delegate: RoundedRadioButton {
                                text: modelData.text
                                checked: modelData.value === bendsModel.guitarDivesAboveStaff.value
                                onToggled: {
                                    bendsModel.guitarDivesAboveStaff.value = modelData.value
                                }
                            }
                        }

                        StyleControlRowWithReset {
                            label: qsTrc("notation/editstyle/bends", "Line thickness:")
                            controlAreaWidth: 100
                            styleItem: bendsModel.guitarDiveLineWidthTab
                            IncrementalPropertyControl {
                                measureUnitsSymbol: qsTrc("global", "sp")
                                step: 0.01
                                currentValue: bendsModel.guitarDiveLineWidthTab.value
                                onValueEdited: function(newValue) {
                                    bendsModel.guitarDiveLineWidthTab.value = newValue
                                }
                            }
                        }
                    }
                }

                StyledGroupBox {
                    Layout.fillWidth: true
                    title: qsTrc("notation/editstyle/bends", "Whammy bar line")

                    ColumnLayout {
                        spacing: 8
                        width: parent.width

                        RowLayout {
                            spacing: 12

                            StyleControlRowWithReset {
                                label: qsTrc("notation/editstyle/bends", "Text label:")
                                controlAreaWidth: 100
                                styleItem: bendsModel.whammyBarText
                                TextInputField {
                                    width: 100
                                    currentText: bendsModel.whammyBarText.value
                                    onTextEditingFinished: function(newValue) {
                                        bendsModel.whammyBarText.value = newValue
                                    }
                                }
                            }

                            SeparatorLine {
                                orientation: Qt.Vertical
                            }

                            FlatButton {
                                text: qsTrc("notation", "Edit text style")

                                onClicked: {
                                    root.goToTextStylePage("whammy-bar")
                                }
                            }
                        }



                        LineStyleSection {
                            id: lineStyleSection

                            lineStyle: bendsModel.whammyBarLineStyle
                            dashLineLength: bendsModel.whammyBarDashLineLen
                            dashGapLength: bendsModel.whammyBarDashGapLen
                            lineWidth: bendsModel.whammyBarLineWidth
                        }
                    }
                }
            }
        }

        StyledGroupBox {
            Layout.fillWidth: true
            Layout.minimumWidth: 500
            title: qsTrc("notation/editstyle/bends", "Grace note bends/dives on tablature")

            ColumnLayout {
                spacing: 12
                width: parent.width

                CheckBox {
                    text: qsTrc("notation/editstyle/bends", "Use cue sized fret numbers")
                    checked: bendsModel.useCueSizeFretForGraceBends.value === true
                    onClicked: bendsModel.useCueSizeFretForGraceBends.value = !bendsModel.useCueSizeFretForGraceBends.value
                }

                ColumnLayout {
                    spacing: 8
                    width: parent.width

                    StyledTextLabel {
                        horizontalAlignment: Text.AlignLeft
                        text: qsTrc("notation/editstyle/bends", "Align pre-bends and pre-dives")
                    }

                    RadioButtonGroup {
                        orientation: Qt.Vertical
                        spacing: 8
                        model: [
                            { text: qsTrc("notation/editstyle/bends", "To the main note"), value: false },
                            { text: qsTrc("notation/editstyle/bends", "To the grace note"), value: true }
                        ]

                        delegate: RoundedRadioButton {
                            text: modelData.text
                            checked: modelData.value === bendsModel.alignPreBendAndPreDiveToGraceNote.value
                            onToggled: {
                                bendsModel.alignPreBendAndPreDiveToGraceNote.value = modelData.value
                            }
                        }
                    }
                }
            }
        }

        StyledGroupBox {
            Layout.fillWidth: true
            Layout.minimumWidth: 500
            title: qsTrc("notation/editstyle/bends", "Interval labels")

            RowLayout {
                spacing: 12

                RadioButtonGroup {
                    Layout.fillWidth: false
                    model: [
                        { iconCode: IconCode.FRACTION_DIAGONAL, text: qsTrc("notation/editstyle/bends", "Diagonal"), value: true },
                        { iconCode: IconCode.FRACTION_LEVEL, text: qsTrc("notation/editstyle/bends", "Level"), value: false }
                    ]

                    delegate: FlatRadioButton {
                        width: 132
                        height: 52

                        Column {
                            anchors.centerIn: parent
                            spacing: 4

                            StyledIconLabel {
                                anchors.horizontalCenter: parent.horizontalCenter
                                iconCode: modelData.iconCode
                                //font.pixelSize: 32
                            }

                            StyledTextLabel {
                                anchors.horizontalCenter: parent.horizontalCenter
                                text: modelData.text
                            }
                        }

                        checked: modelData.value === bendsModel.useFractionCharacters.value

                        onToggled: {
                            bendsModel.useFractionCharacters.value = modelData.value
                        }
                    }
                }

                SeparatorLine {
                    orientation: Qt.Vertical
                }

                FlatButton {
                    text: qsTrc("notation", "Edit text style")

                    onClicked: {
                        root.goToTextStylePage("bend")
                    }
                }
            }
        }
    }
}
