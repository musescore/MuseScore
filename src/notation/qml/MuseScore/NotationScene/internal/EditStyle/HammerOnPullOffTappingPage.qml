/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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
import QtQuick.Controls

import MuseScore.NotationScene 1.0
import Muse.UiComponents 1.0
import Muse.Ui 1.0

StyledFlickable {
    id: root

    signal goToTextStylePage(string s)

    contentWidth: column.width
    contentHeight: column.height

    HammerOnPullOffTappingPageModel {
        id: hopoPage
    }

    ColumnLayout {
        id: column
        spacing: 12

        StyledGroupBox {
            Layout.fillWidth: true
            Layout.minimumWidth: 500
            title: qsTrc("notation/editstyle/hammeronpulloff", "Visibility")

            ColumnLayout {
                width: parent.width
                spacing: 10

                StyledTextLabel {
                    text: qsTrc("notation/editstyle/hammeronpulloff", "Show ‘H’ and ‘P’ symbols on")
                }

                RowLayout {
                    ToggleButton {
                        checked: hopoPage.showOnStandardStaves.value === true
                        onToggled: {
                            hopoPage.showOnStandardStaves.value = !hopoPage.showOnStandardStaves.value
                        }
                    }

                    StyledTextLabel {
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignLeft
                        text: "Standard staves"
                    }
                }

                RowLayout {
                    ToggleButton {
                        checked: hopoPage.showOnTabStaves.value === true
                        onToggled: {
                            hopoPage.showOnTabStaves.value = !hopoPage.showOnTabStaves.value
                        }
                    }

                    StyledTextLabel {
                        id : toggleText
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignLeft
                        text: "Tablature staves"
                    }
                }
            }
        }

        StyledGroupBox {
            Layout.fillWidth: true
            Layout.minimumWidth: 500
            title: qsTrc("notation/editstyle/hammeronpulloff", "Case")

            RowLayout {
                width: parent.width
                spacing: 10

                RadioButtonGroup {
                    orientation: ListView.Horizontal
                    spacing: 6

                    model: [
                        {iconCode: IconCode.HP_UPPER_CASE, value: true },
                        {iconCode: IconCode.HP_LOWER_CASE, value: false },
                    ]

                    delegate: FlatRadioButton {
                        width: 40
                        height: 30
                        iconCode: modelData.iconCode
                        checked: hopoPage.hopoUpperCase.value === modelData.value
                        onToggled: hopoPage.hopoUpperCase.value = modelData.value
                    }
                }

                SeparatorLine {
                    orientation: Qt.Vertical
                }

                FlatButton {
                    text: qsTrc("notation", "Edit text style")

                    onClicked: {
                        root.goToTextStylePage("hammer-ons-pull-offs-and-tapping")
                    }
                }

                Item {
                    Layout.fillWidth: true
                }
            }
        }

        StyledGroupBox {
            Layout.fillWidth: true
            Layout.minimumWidth: 500
            title: qsTrc("notation/editstyle/hammeronpulloff", "Consecutive hammer-on/pull-offs")

            ColumnLayout {
                width: parent.width
                spacing: 10

                RadioButtonGroup {
                    orientation: ListView.Vertical
                    spacing: 6

                    model: [
                        {text: qsTrc("notation/editstyle/hammeronpulloff", "Show ‘H’ or ‘P’ between each pair of notes"), value: true },
                        {text: qsTrc("notation/editstyle/hammeronpulloff", "Show ‘H’ or ‘P’ once per group of ascending/descending notes"), value: false },
                    ]

                    delegate: RoundedRadioButton {
                        text: modelData.text
                        checked: hopoPage.hopoShowAll.value === modelData.value
                        onToggled: hopoPage.hopoShowAll.value = modelData.value
                    }
                }

                StyledImage {
                    forceHeight: 180
                    source: hopoPage.hopoShowAll.value === true ? "hammerOnPullOffImages/hopoShowAll" : "hammerOnPullOffImages/hopoNotShowAll"
                }
            }
        }

        StyledGroupBox {
            Layout.fillWidth: true
            Layout.minimumWidth: 500
            title: qsTrc("notation/editstyle/hammeronpulloff", "Left-hand tapping")

            ColumnLayout {
                width: parent.width
                spacing: 10

                ColumnLayout {
                    width: parent.width
                    spacing: 8

                    StyledTextLabel {
                        text: qsTrc("notation/editstyle/hammeronpulloff", "Symbol")
                    }

                    RadioButtonGroup {
                        orientation: ListView.Horizontal
                        spacing: 6

                        model: [
                            {iconCode: IconCode.TAPPING_ENCIRCLED_T, width: 40, value: 2 },
                            {iconCode: IconCode.TAPPING_DOT, width: 40, value: 1 },
                            {text: qsTrc("notation/editstyle/hammeronpulloff", "None"), width: 53,  value: 0 },
                        ]

                        delegate: FlatRadioButton {
                            width: modelData.width
                            height: 30
                            text: modelData.text
                            iconCode: modelData.iconCode
                            checked: hopoPage.lhTappingSymbol.value === modelData.value
                            onToggled: hopoPage.lhTappingSymbol.value = modelData.value
                        }
                    }
                }

                ColumnLayout {
                    width: parent.width
                    spacing: 8

                    StyledTextLabel {
                        text: qsTrc("notation/editstyle/hammeronpulloff", "Show half slurs on")
                    }

                    RowLayout {
                        ToggleButton {
                            checked: hopoPage.lhTappingShowHalfSlursOnNormalStave.value === true
                            onToggled: {
                                hopoPage.lhTappingShowHalfSlursOnNormalStave.value = !hopoPage.lhTappingShowHalfSlursOnNormalStave.value
                            }
                        }

                        StyledTextLabel {
                            Layout.fillWidth: true
                            horizontalAlignment: Text.AlignLeft
                            text: qsTrc("notation/editstyle/hammeronpulloff", "Standard staves")
                        }
                    }

                    RowLayout {
                        ToggleButton {
                            checked: hopoPage.lhTappingShowHalfSlursOnTab.value === true
                            onToggled: {
                                hopoPage.lhTappingShowHalfSlursOnTab.value = !hopoPage.lhTappingShowHalfSlursOnTab.value
                            }
                        }

                        StyledTextLabel {
                            Layout.fillWidth: true
                            horizontalAlignment: Text.AlignLeft
                            text: qsTrc("notation/editstyle/hammeronpulloff", "Tablature staves")
                        }
                    }
                }

                CheckBox {
                    text: qsTrc("notation/editstyle/hammeronpulloff", "Slur top and bottom notes in chords (tablature only)")
                    checked: hopoPage.lhTappingSlurTopAndBottomNoteOnTab.value === true
                    onClicked: hopoPage.lhTappingSlurTopAndBottomNoteOnTab.value = !hopoPage.lhTappingSlurTopAndBottomNoteOnTab.value
                }
            }
        }

        StyledGroupBox {
            Layout.fillWidth: true
            Layout.minimumWidth: 500
            title: qsTrc("notation/editstyle/hammeronpulloff", "Right-hand tapping")

            ColumnLayout {
                width: parent.width
                spacing: 10

                ColumnLayout {
                    width: parent.width
                    spacing: 8

                    StyledTextLabel {
                        text: qsTrc("notation/editstyle/hammeronpulloff", "Symbol (standard stave)")
                    }

                    RadioButtonGroup {
                        orientation: ListView.Horizontal
                        spacing: 6

                        model: [
                            {iconCode: IconCode.TAPPING_PLUS, value: 2, width: 40 },
                            {iconCode: IconCode.TAPPING_T, value: 1, width: 40 },
                            {text: qsTrc("notation/editstyle/hammeronpulloff", "None"), value: 0, width: 52},
                        ]

                        delegate: FlatRadioButton {
                            height: 30
                            width: modelData.width
                            text: modelData.text
                            iconCode: modelData.iconCode
                            checked: hopoPage.rhTappingSymbolNormalStave.value === modelData.value
                            onToggled: hopoPage.rhTappingSymbolNormalStave.value = modelData.value
                        }
                    }
                }

                ColumnLayout {
                    width: parent.width
                    spacing: 8

                    StyledTextLabel {
                        text: qsTrc("notation/editstyle/hammeronpulloff", "Symbol (tablature stave)")
                    }

                    RadioButtonGroup {
                        orientation: ListView.Horizontal
                        spacing: 6

                        model: [
                            {iconCode: IconCode.TAPPING_PLUS, value: 2, width: 40 },
                            {iconCode: IconCode.TAPPING_T, value: 1, width: 40 },
                            {text: qsTrc("notation/editstyle/hammeronpulloff", "None"), value: 0, width: 52},
                        ]

                        delegate: FlatRadioButton {
                            height: 30
                            width: modelData.width
                            text: modelData.text
                            iconCode: modelData.iconCode
                            checked: hopoPage.rhTappingSymbolTab.value === modelData.value
                            onToggled: hopoPage.rhTappingSymbolTab.value = modelData.value
                        }
                    }
                }
            }
        }
    }
}
