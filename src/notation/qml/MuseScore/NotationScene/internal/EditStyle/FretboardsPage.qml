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
            spacing: 12
            width: parent.width

            BasicStyleSelectorWithSpinboxAndReset {
                styleItem: fretboardsPage.fretY
                label: qsTrc("notation", "Position above:")
                suffix: qsTrc("global", "sp")
                controlAreaWidth: 204
            }

            BasicStyleSelectorWithSpinboxAndReset {
                styleItem: fretboardsPage.fretMag
                label: qsTrc("notation", "Scale:")
                inPercentage: true
                controlAreaWidth: 204
            }

            IconAndTextButtonSelector {
                styleItem: fretboardsPage.fretOrientation
                label: qsTrc("notation", "Orientation:")

                Layout.preferredHeight: 70

                model: [
                    { iconCode: IconCode.FRETBOARD_VERTICAL, text: qsTrc("notation", "Vertical"), value: 0 },
                    { iconCode: IconCode.FRETBOARD_HORIZONTAL, text: qsTrc("notation", "Horizontal"), value: 1 }
                ]
            }

            BasicStyleSelectorWithSpinboxAndReset {
                styleItem: fretboardsPage.fretNutThickness
                label: qsTrc("notation", "Nut line thickness:")
                suffix: qsTrc("global", "sp")
                controlAreaWidth: 204
            }

            StyledGroupBox {
                Layout.fillWidth: true

                title: qsTrc("notation", "Fret number")

                ColumnLayout {
                    width: parent.width
                    spacing: 12

                    RowLayout {
                        spacing: 12

                        StyledTextLabel {
                            horizontalAlignment: Text.AlignLeft
                            text: qsTrc("notation", "Position:")
                        }

                        RoundedRadioButton {
                            text: fretboardsPage.fretOrientation.value === 0 ? qsTrc("notation", "Left") : qsTrc("notation", "Bottom")
                            checked: fretboardsPage.fretNumPos.value === 0
                            onToggled: fretboardsPage.fretNumPos.value = 0
                        }

                        RoundedRadioButton {
                            text: fretboardsPage.fretOrientation.value === 0 ? qsTrc("notation", "Right") : qsTrc("notation", "Top")
                            checked: fretboardsPage.fretNumPos.value === 1
                            onToggled: fretboardsPage.fretNumPos.value = 1
                        }
                    }

                    StyledGroupBox {
                        title: qsTrc("notation", "Format:")
                        Layout.fillWidth: true

                        ColumnLayout {
                            width: parent.width
                            spacing: 12

                            RoundedRadioButton {
                                text: qsTrc("notation", "Number only")
                                checked: fretboardsPage.fretUseCustomSuffix.value === false
                                onToggled: fretboardsPage.fretUseCustomSuffix.value = false
                            }

                            RowLayout {
                                spacing: 8

                                RoundedRadioButton {
                                    text: qsTrc("notation", "Custom suffix:")
                                    checked: fretboardsPage.fretUseCustomSuffix.value === true
                                    onToggled: fretboardsPage.fretUseCustomSuffix.value = true
                                }

                                TextInputField {
                                    Layout.preferredWidth: 60
                                    enabled: fretboardsPage.fretUseCustomSuffix.value === true
                                    currentText: fretboardsPage.fretCustomSuffix.value
                                    onTextEdited: function(newTextValue) {
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

                    FlatButton {
                        text: qsTrc("notation", "Edit fret number text style")

                        onClicked: {
                            root.goToTextStylePage("fretboard-diagram-fret-number")
                        }
                    }
                }
            }

            BasicStyleSelectorWithSpinboxAndReset {
                styleItem: fretboardsPage.fretDotSpatiumSize
                label: qsTrc("notation", "Dot size:")
                suffix: qsTrc("global", "sp")
                controlAreaWidth: 204

            }

            StyledGroupBox {
                Layout.fillWidth: true

                title: qsTrc("notation", "Barré")

                ColumnLayout {
                    width: parent.width
                    spacing : 12

                    IconAndTextButtonSelector {
                        styleItem: fretboardsPage.barreAppearanceSlur
                        label: qsTrc("notation", "Appearance:")

                        Layout.preferredHeight: 70

                        model: [
                            { iconCode: IconCode.FRETBOARD_BARRE_LINE, text: qsTrc("notation", "Line"), value: false },
                            { iconCode: IconCode.FRETBOARD_BARRE_SLUR, text: qsTrc("notation", "Slur"), value: true }
                        ]
                    }

                    BasicStyleSelectorWithSpinboxAndReset {
                        styleItem: fretboardsPage.barreLineWidth
                        label: qsTrc("notation", "Line thickness:")
                        inPercentage: true
                        controlAreaWidth: 204
                    }
                }
            }

            CheckBox {
                text: qsTrc("notation", "Show fingerings")
                checked: fretboardsPage.fretShowFingerings.value === true
                onClicked: fretboardsPage.fretShowFingerings.value = !fretboardsPage.fretShowFingerings.value
            }

            FlatButton {
                text: qsTrc("notation", "Edit fingering text style")

                onClicked: {
                    root.goToTextStylePage("fretboard-diagram-fingering")
                }
            }

            IconAndTextButtonSelector {
                styleItem: fretboardsPage.fretStyleExtended
                label: qsTrc("notation", "Fretboard style:")

                Layout.preferredHeight: 70

                model: [
                    { iconCode: IconCode.FRETBOARD_VERTICAL, text: qsTrc("notation", "Trimmed"), value: false },
                    { iconCode: IconCode.FRETBOARD_EXTENDED, text: qsTrc("notation", "Extended"), value: true }
                ]
            }

            BasicStyleSelectorWithSpinboxAndReset {
                styleItem: fretboardsPage.fretStringSpacing
                label: qsTrc("notation", "String spacing:")
                suffix: qsTrc("global", "sp")
                controlAreaWidth: 204
            }

            BasicStyleSelectorWithSpinboxAndReset {
                styleItem: fretboardsPage.fretFretSpacing
                label: qsTrc("notation", "Fret spacing:")
                suffix: qsTrc("global", "sp")
                controlAreaWidth: 204
            }

            BasicStyleSelectorWithSpinboxAndReset {
                styleItem: fretboardsPage.maxFretShiftAbove
                label: qsTrc("notation", "Maximum shift above:")
                suffix: qsTrc("global", "sp")
                controlAreaWidth: 204
            }

            BasicStyleSelectorWithSpinboxAndReset {
                styleItem: fretboardsPage.maxFretShiftBelow
                label: qsTrc("notation", "Maximum shift below:")
                suffix: qsTrc("global", "sp")
                controlAreaWidth: 204
            }
        }
    }
}
