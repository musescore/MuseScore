/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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
import QtQuick.Layouts
import QtQuick.Controls

import Muse.Ui
import Muse.UiComponents
import MuseScore.NotationScene

StyledFlickable {
    id: root

    signal goToTextStylePage(int index)

    contentWidth: Math.max(column.implicitWidth, root.width)
    contentHeight: column.implicitHeight

    InstrumentNamesPageModel {
        id: instrumentNamesModel
    }

    ColumnLayout {
        id: column
        width: parent.width
        spacing: 12

        StyledGroupBox {
            Layout.fillWidth: true
            Layout.minimumWidth: 500
            title: qsTrc("notation/editstyle/instrumentnames", "Transposition")

            ColumnLayout {
                spacing: 12

                ColumnLayout {
                    spacing: 8

                    StyledTextLabel {
                        text: qsTrc("notation/editstyle/instrumentnames", "Show transposition")
                    }

                    StyleToggle {
                        text: qsTrc("notation/editstyle/instrumentnames", "On long names")
                        styleItem: instrumentNamesModel.instrumentNamesShowTranspositionLong
                    }

                    StyleToggle {
                        text: qsTrc("notation/editstyle/instrumentnames", "On abbreviated names")
                        styleItem: instrumentNamesModel.instrumentNamesShowTranspositionShort
                    }
                }

                ColumnLayout {
                    spacing: 8

                    enabled: instrumentNamesModel.instrumentNamesShowTranspositionLong.value === true

                    StyledTextLabel {
                        text: qsTrc("notation/editstyle/instrumentnames", "Long name format:")
                    }

                    ColumnLayout {
                        spacing : 8
                        Layout.alignment: Qt.AlignTop

                        Repeater {
                            model: [
                                { text: qsTrc("notation/editstyle/instrumentnames", "Horn in F 1"), value: 0 },
                                { text: qsTrc("notation/editstyle/instrumentnames", "Horn 1 in F"), value: 1 },
                                { text: qsTrc("notation/editstyle/instrumentnames", "F Horn 1"), value: 2 },
                                { text: qsTrc("notation/editstyle/instrumentnames", "Custom:"), value: 3 },
                            ]

                            RoundedRadioButton {
                                required property var modelData
                                text: modelData.text
                                checked: instrumentNamesModel.instrumentNamesFormatLong.value === modelData.value
                                onClicked: instrumentNamesModel.instrumentNamesFormatLong.value = modelData.value
                            }
                        }
                    }

                    RowLayout {
                        id: customFormatLong
                        spacing: 8

                        enabled: instrumentNamesModel.instrumentNamesFormatLong.value === 3

                        TextInputField {
                            Layout.preferredWidth: 220
                            currentText: instrumentNamesModel.instrumentNamesCustomFormatLong.value
                            onTextEditingFinished: function(newValue) {
                                instrumentNamesModel.instrumentNamesCustomFormatLong.value = newValue
                            }
                        }

                        StyleResetButton {
                            styleItem: instrumentNamesModel.instrumentNamesCustomFormatLong
                            enabled: !styleItem.isDefault && customFormatLong.enabled
                        }
                    }
                }

                ColumnLayout {
                    spacing: 8

                    enabled: instrumentNamesModel.instrumentNamesShowTranspositionShort.value === true

                    StyledTextLabel {
                        text: qsTrc("notation/editstyle/instrumentnames", "Short name format:")
                    }

                    ColumnLayout {
                        spacing : 8
                        Layout.alignment: Qt.AlignTop

                        Repeater {
                            model: [
                                { text: qsTrc("notation/editstyle/instrumentnames", "Hn. in F 1"), value: 0 },
                                { text: qsTrc("notation/editstyle/instrumentnames", "Hn. 1 in F"), value: 1 },
                                { text: qsTrc("notation/editstyle/instrumentnames", "F Hn. 1"), value: 2 },
                                { text: qsTrc("notation/editstyle/instrumentnames", "Custom:"), value: 3 },
                            ]

                            RoundedRadioButton {
                                required property var modelData
                                text: modelData.text
                                checked: instrumentNamesModel.instrumentNamesFormatShort.value === modelData.value
                                onClicked: instrumentNamesModel.instrumentNamesFormatShort.value = modelData.value
                            }
                        }
                    }

                    RowLayout {
                        id: customFormatShort
                        spacing: 8

                        enabled: instrumentNamesModel.instrumentNamesFormatShort.value === 3

                        TextInputField {
                            Layout.preferredWidth: 220
                            currentText: instrumentNamesModel.instrumentNamesCustomFormatShort.value
                            onTextEditingFinished: function(newValue) {
                                instrumentNamesModel.instrumentNamesCustomFormatShort.value = newValue
                            }
                        }

                        StyleResetButton {
                            styleItem: instrumentNamesModel.instrumentNamesCustomFormatShort
                            enabled: !styleItem.isDefault && customFormatShort.enabled
                        }
                    }
                }
            }
        }

        StyledGroupBox {
            Layout.fillWidth: true
            Layout.minimumWidth: 500
            title: qsTrc("notation/editstyle/instrumentnames", "Group names")

            ColumnLayout {
                spacing: 12

                ColumnLayout {
                    spacing: 8

                    StyledTextLabel {
                        text: qsTrc("notation/editstyle/instrumentnames", "Enable group names for")
                    }

                    StyleToggle {
                        text: qsTrc("notation/editstyle/instrumentnames", "Winds")
                        styleItem: instrumentNamesModel.windsNameByGroup
                    }

                    StyleToggle {
                        text: qsTrc("notation/editstyle/instrumentnames", "Vocals")
                        styleItem: instrumentNamesModel.vocalsNameByGroup
                    }

                    StyleToggle {
                        text: qsTrc("notation/editstyle/instrumentnames", "Strings")
                        styleItem: instrumentNamesModel.stringsNameByGroup
                    }

                    StyleToggle {
                        text: qsTrc("notation/editstyle/instrumentnames", "Others")
                        styleItem: instrumentNamesModel.othersNameByGroup
                    }
                }
            }
        }

        StyledGroupBox {
            Layout.fillWidth: true
            Layout.minimumWidth: 500
            title: qsTrc("notation/editstyle/instrumentnames", "Alignment")

            ColumnLayout {
                width: parent.width
                spacing: 12

                ColumnLayout {
                    width: parent.width
                    spacing: 8

                    StyledTextLabel {
                        text: qsTrc("notation/editstyle/instrumentnames", "Long names")
                    }

                    RowLayout {
                        spacing: 8

                        StyledImage {
                            Layout.alignment: Qt.AlignTop
                            forceHeight: 120
                            source: switch(instrumentNamesModel.instrumentNamesAlignLong.value) {
                                case 0: "instrumentNamesImages/long-right-right.png"; break;
                                case 1: "instrumentNamesImages/long-center-right.png"; break;
                                case 2: "instrumentNamesImages/long-center-center.png"; break;
                                case 3: "instrumentNamesImages/long-left-right.png"; break;
                            }
                        }

                        ColumnLayout {
                            spacing : 8
                            Layout.alignment: Qt.AlignTop

                            Repeater {
                                model: [
                                    { text: qsTrc("notation/editstyle/instrumentnames", "Instrument name: right-align / Staff label: right-align"), value: 0 },
                                    { text: qsTrc("notation/editstyle/instrumentnames", "Instrument name: center / Staff label: right-align"), value: 1 },
                                    { text: qsTrc("notation/editstyle/instrumentnames", "Instrument name: center / Staff label: center"), value: 2 },
                                    { text: qsTrc("notation/editstyle/instrumentnames", "Instrument name: left-align / Staff label: right-align"), value: 3 },
                                ]

                                RoundedRadioButton {
                                    required property var modelData
                                    text: modelData.text
                                    checked: instrumentNamesModel.instrumentNamesAlignLong.value === modelData.value
                                    onClicked: instrumentNamesModel.instrumentNamesAlignLong.value = modelData.value
                                }
                            }
                        }
                    }
                }

                ColumnLayout {
                    width: parent.width
                    spacing: 8

                    StyledTextLabel {
                        text: qsTrc("notation/editstyle/instrumentnames", "Abbreviated names")
                    }

                    RowLayout {
                        spacing: 8

                        StyledImage {
                            Layout.alignment: Qt.AlignTop
                            forceHeight: 120
                            source: switch(instrumentNamesModel.instrumentNamesAlignShort.value) {
                                case 0: "instrumentNamesImages/short-right-right.png"; break;
                                case 1: "instrumentNamesImages/short-center-right.png"; break;
                                case 2: "instrumentNamesImages/short-center-center.png"; break;
                                case 3: "instrumentNamesImages/short-left-right.png"; break;
                            }
                        }

                        ColumnLayout {
                            spacing : 8
                            Layout.alignment: Qt.AlignTop

                            Repeater {
                                model: [
                                    { text: qsTrc("notation/editstyle/instrumentnames", "Instrument name: right-align / Staff label: right-align"), value: 0 },
                                    { text: qsTrc("notation/editstyle/instrumentnames", "Instrument name: center / Staff label: right-align"), value: 1 },
                                    { text: qsTrc("notation/editstyle/instrumentnames", "Instrument name: center / Staff label: center"), value: 2 },
                                    { text: qsTrc("notation/editstyle/instrumentnames", "Instrument name: left-align / Staff label: right-align"), value: 3 },
                                ]

                                RoundedRadioButton {
                                    required property var modelData
                                    text: modelData.text
                                    checked: instrumentNamesModel.instrumentNamesAlignShort.value === modelData.value
                                    onClicked: instrumentNamesModel.instrumentNamesAlignShort.value = modelData.value
                                }
                            }
                        }
                    }
                }

                ColumnLayout {
                    width: parent.width
                    spacing: 8
                    enabled: instrumentNamesModel.instrumentNamesAlignLong.value === 0 || instrumentNamesModel.instrumentNamesAlignShort.value === 0

                    StyledTextLabel {
                        text: qsTrc("notation/editstyle/instrumentnames", "Overlapping names")
                    }

                    RowLayout {
                        spacing: 8

                        StyledImage {
                            Layout.alignment: Qt.AlignTop
                            forceHeight: 170
                            source: instrumentNamesModel.instrumentNamesStackVertically.value === true ? "instrumentNamesImages/stack-vertically.png" : "instrumentNamesImages/stack-horizontally.png"
                        }

                        ColumnLayout {
                            spacing : 8
                            Layout.alignment: Qt.AlignTop

                            Repeater {
                                model: [
                                    { text: qsTrc("notation/editstyle/instrumentnames", "Keep on the same line"), value: false },
                                    { text: qsTrc("notation/editstyle/instrumentnames", "Stack vertically"), value: true },
                                ]

                                RoundedRadioButton {
                                    required property var modelData
                                    text: modelData.text
                                    checked: instrumentNamesModel.instrumentNamesStackVertically.value === modelData.value
                                    onClicked: instrumentNamesModel.instrumentNamesStackVertically.value = modelData.value
                                }
                            }
                        }
                    }
                }

                ColumnLayout {
                    width: parent.width
                    spacing: 8

                    StyledTextLabel {
                        text: qsTrc("notation/editstyle/instrumentnames", "Alignment of names and group brackets")
                    }

                    RowLayout {
                        spacing: 8

                        StyledImage {
                            Layout.alignment: Qt.AlignTop
                            forceHeight: 145
                            source: instrumentNamesModel.instrumentNamesAlignIncludeGroupBrackets.value === true ? "instrumentNamesImages/groupBracket-align-include.png" : "instrumentNamesImages/groupBracket-align-exclude.png"
                        }

                        ColumnLayout {
                            spacing : 8
                            Layout.alignment: Qt.AlignTop

                            Repeater {
                                model: [
                                    { text: qsTrc("notation/editstyle/instrumentnames", "Include brackets"), value: true },
                                    { text: qsTrc("notation/editstyle/instrumentnames", "Exclude brackets"), value: false },
                                ]

                                RoundedRadioButton {
                                    required property var modelData
                                    text: modelData.text
                                    checked: instrumentNamesModel.instrumentNamesAlignIncludeGroupBrackets.value === modelData.value
                                    onClicked: instrumentNamesModel.instrumentNamesAlignIncludeGroupBrackets.value = modelData.value
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
