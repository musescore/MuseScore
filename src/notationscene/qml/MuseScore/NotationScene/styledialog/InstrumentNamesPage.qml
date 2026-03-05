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
            title: qsTrc("notation/editstyle/instrumentnames", "Instrument names")

            ColumnLayout {
                spacing: 12

                ColumnLayout {
                    spacing: 8

                    StyledTextLabel {
                        text: qsTrc("notation/editstyle/instrumentnames", "Enable name grouping for")
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
            }
        }
    }
}
