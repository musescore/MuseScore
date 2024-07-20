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
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

import MuseScore.NotationScene 1.0
import Muse.UiComponents 1.0
import Muse.Ui 1.0

Rectangle {
    id: root
    anchors.fill: parent
    color: ui.theme.backgroundPrimaryColor

    signal goToTextStylePage(string s)

    StyledGroupBox {
        width: parent.width
        height: parent.height

        title: qsTrc("notation", "Fretboard diagrams")

        FretboardsPageModel {
            id: fretboardsPage
        }

        ColumnLayout {
            width: parent.width

            spacing: 12

            BasicStyleSelectorWithSpinboxAndReset {
                styleItem: fretboardsPage.fretY
                label: qsTrc("notation", "Position above:")
                suffix: qsTrc("global", "sp")
            }

            BasicStyleSelectorWithSpinboxAndReset {
                styleItem: fretboardsPage.fretMag
                label: qsTrc("notation", "Scale:")
                inPercentage: true
            }

            RowLayout {
                StyledTextLabel {
                    Layout.fillWidth: true
                    horizontalAlignment: Qt.AlignLeft
                    text: qsTrc("notation", "Orientation:")
                }

                RadioButtonGroup {
                    Layout.preferredHeight: 70

                    model: [
                        { iconCode: IconCode.FRETBOARD_VERTICAL, text: qsTrc("notation", "Vertical"), value: 0 },
                        { iconCode: IconCode.FRETBOARD_HORIZONTAL, text: qsTrc("notation", "Horizontal"), value: 1 }
                    ]

                    delegate: FlatRadioButton {
                        width: 100
                        height: 70
                        Column {
                            anchors.centerIn: parent
                            height: childrenRect.height
                            spacing: 8
                            StyledIconLabel {
                                anchors.horizontalCenter: parent.horizontalCenter
                                iconCode: modelData.iconCode
                                font.pixelSize: 28
                            }
                            StyledTextLabel {
                                anchors.horizontalCenter: parent.horizontalCenter
                                text: modelData.text
                            }
                        }
                        checked: fretboardsPage.fretOrientation.value === modelData.value
                        onToggled: fretboardsPage.fretOrientation.value = modelData.value
                    }
                }
            }

            BasicStyleSelectorWithSpinboxAndReset {
                styleItem: fretboardsPage.fretNutThickness
                label: qsTrc("notation", "Nut line thickness:")
                suffix: qsTrc("global", "sp")
            }

            StyledGroupBox {
                Layout.fillWidth: true

                title: qsTrc("notation", "Fret number")
                ColumnLayout {
                    width: parent.width
                    RowLayout {
                        spacing: 12
                        StyledTextLabel {
                            horizontalAlignment: Text.AlignLeft
                            text: qsTrc("notation", "Position:")
                        }
                        RowLayout {
                            spacing: 8
                            RoundedRadioButton {
                                checked: fretboardsPage.fretNumPos.value === 0
                                onToggled: fretboardsPage.fretNumPos.value = 0
                            }
                            StyledTextLabel {
                                horizontalAlignment: Text.AlignLeft
                                text: fretboardsPage.fretOrientation.value === 0 ? qsTrc("notation", "Left") : qsTrc("notation", "Bottom")
                            }
                        }
                        RowLayout {
                            spacing: 8
                            RoundedRadioButton {
                                checked: fretboardsPage.fretNumPos.value === 1
                                onToggled: fretboardsPage.fretNumPos.value = 1
                            }
                            StyledTextLabel {
                                horizontalAlignment: Text.AlignLeft
                                text: fretboardsPage.fretOrientation.value === 0 ? qsTrc("notation", "Right") : qsTrc("notation", "Top")
                            }
                        }
                    }

                    StyledGroupBox {
                        title: qsTrc("notation", "Format:")
                        Layout.fillWidth: true
                        ColumnLayout {
                            RowLayout {
                                spacing: 8
                                RoundedRadioButton {
                                    checked: fretboardsPage.fretUseCustomSuffix.value === false
                                    onToggled: fretboardsPage.fretUseCustomSuffix.value = false
                                }
                                StyledTextLabel {
                                    horizontalAlignment: Text.AlignLeft
                                    text: qsTrc("notation", "Number only")
                                }
                            }
                            RowLayout {
                                spacing: 8
                                RoundedRadioButton {
                                    checked: fretboardsPage.fretUseCustomSuffix.value === true
                                    onToggled: fretboardsPage.fretUseCustomSuffix.value = true
                                }
                                StyledTextLabel {
                                    horizontalAlignment: Text.AlignLeft
                                    text: qsTrc("notation", "Custom suffix:")
                                }
                                TextInputField {
                                    enabled: fretboardsPage.fretUseCustomSuffix.value === true
                                    Layout.preferredWidth: 60
                                    currentText: fretboardsPage.fretCustomSuffix.value
                                    onTextChanged: function(newTextValue) {
                                        fretboardsPage.fretCustomSuffix.value = newTextValue
                                    }
                                }
                                StyledTextLabel {
                                    visible: fretboardsPage.fretUseCustomSuffix.value === true
                                    horizontalAlignment: Text.AlignLeft
                                    text: qsTrc("notation", "Preview:") + " 3" + fretboardsPage.fretCustomSuffix.value
                                }
                            }
                        }
                    }

                    FlatRadioButton {
                        Layout.preferredWidth: 160
                        text: qsTrc("notation", "Edit fret number text style")
                        checked: false
                        onClicked: function() {
                            checked = false
                            root.goToTextStylePage("fretboard-diagram-fret-number")
                        }
                    }
                }
            }

            BasicStyleSelectorWithSpinboxAndReset {
                styleItem: fretboardsPage.fretDotSpatiumSize
                label: qsTrc("notation", "Dot size:")
                suffix: qsTrc("global", "sp")
            }

            StyledGroupBox {
                Layout.fillWidth: true

                title: qsTrc("notation", "Barré")

                ColumnLayout {
                    spacing : 12
                    width: parent.width
                    height: parent.height
                    RowLayout {
                        Layout.preferredHeight: 70
                        width: parent.width

                        StyledTextLabel {
                            Layout.fillWidth: true
                            horizontalAlignment: Text.AlignLeft
                            text: qsTrc("notation", "Appearance:")
                        }

                        RadioButtonGroup {
                            Layout.preferredHeight: 70

                            model: [
                                { iconCode: IconCode.FRETBOARD_BARRE_LINE, text: qsTrc("notation", "Line"), value: false },
                                { iconCode: IconCode.FRETBOARD_BARRE_SLUR, text: qsTrc("notation", "Slur"), value: true }
                            ]

                            delegate: FlatRadioButton {
                                width: 100
                                height: 70
                                Column {
                                    anchors.centerIn: parent
                                    height: childrenRect.height
                                    spacing: 8
                                    StyledIconLabel {
                                        anchors.horizontalCenter: parent.horizontalCenter
                                        iconCode: modelData.iconCode
                                        font.pixelSize: 28
                                    }
                                    StyledTextLabel {
                                        anchors.horizontalCenter: parent.horizontalCenter
                                        text: modelData.text
                                    }
                                }
                                checked: fretboardsPage.barreAppearanceSlur.value === modelData.value
                                onToggled: fretboardsPage.barreAppearanceSlur.value = modelData.value
                            }
                        }
                    }

                    BasicStyleSelectorWithSpinboxAndReset {
                        styleItem: fretboardsPage.barreLineWidth
                        label: qsTrc("notation", "Line thickness:")
                        inPercentage: true
                    }
                }
            }

            CheckBox {
                text: qsTrc("notation", "Show fingerings")
                checked: fretboardsPage.fretShowFingerings.value === true
                onClicked: fretboardsPage.fretShowFingerings.value = !fretboardsPage.fretShowFingerings.value
            }

            FlatRadioButton {
                Layout.preferredWidth: 160
                text: qsTrc("notation", "Edit fingering text style")
                checked: false
                onClicked: function() {
                    checked = false
                    root.goToTextStylePage("fretboard-diagram-fingering")
                }
            }

            RowLayout {
                width: parent.width

                StyledTextLabel {
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignLeft
                    text: qsTrc("notation", "Fretboard style:")
                }

                RadioButtonGroup {
                    Layout.preferredHeight: 70

                    model: [
                        { iconCode: IconCode.FRETBOARD_VERTICAL, text: qsTrc("notation", "Trimmed"), value: false },
                        { iconCode: IconCode.FRETBOARD_EXTENDED, text: qsTrc("notation", "Extended"), value: true }
                    ]

                    delegate: FlatRadioButton {
                        width: 100
                        height: 70
                        Column {
                            anchors.centerIn: parent
                            height: childrenRect.height
                            spacing: 8
                            StyledIconLabel {
                                anchors.horizontalCenter: parent.horizontalCenter
                                iconCode: modelData.iconCode
                                font.pixelSize: 28
                            }
                            StyledTextLabel {
                                anchors.horizontalCenter: parent.horizontalCenter
                                text: modelData.text
                            }
                        }
                        checked: fretboardsPage.fretStyleExtended.value === modelData.value
                        onToggled: fretboardsPage.fretStyleExtended.value = modelData.value
                    }
                }
            }

            BasicStyleSelectorWithSpinboxAndReset {
                styleItem: fretboardsPage.fretStringSpacing
                label: qsTrc("notation", "String spacing:")
                suffix: qsTrc("global", "sp")
            }

            BasicStyleSelectorWithSpinboxAndReset {
                styleItem: fretboardsPage.fretFretSpacing
                label: qsTrc("notation", "Fret spacing:")
                suffix: qsTrc("global", "sp")
            }

            BasicStyleSelectorWithSpinboxAndReset {
                styleItem: fretboardsPage.maxFretShiftAbove
                label: qsTrc("notation", "Maximum shift above:")
                suffix: qsTrc("global", "sp")
            }

            BasicStyleSelectorWithSpinboxAndReset {
                styleItem: fretboardsPage.maxFretShiftBelow
                label: qsTrc("notation", "Maximum shift below:")
                suffix: qsTrc("global", "sp")
            }
        }
    }
}
