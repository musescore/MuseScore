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
                    Layout.fillWidth: true
                    text: qsTrc("notation/editstyle/hammeronpulloff", "Show ‘H’ and ‘P’ symbols on")
                    horizontalAlignment: Text.AlignLeft
                }

                ToggleButton {
                    Layout.fillWidth: true
                    text: qsTrc("notation/editstyle/hammeronpulloff", "Standard staves")
                    checked: hopoPage.showOnStandardStaves.value === true
                    onToggled: {
                        hopoPage.showOnStandardStaves.value = !hopoPage.showOnStandardStaves.value
                    }
                }

                ToggleButton {
                    Layout.fillWidth: true
                    text: qsTrc("notation/editstyle/hammeronpulloff", "Tablature staves")
                    checked: hopoPage.showOnTabStaves.value === true
                    onToggled: {
                        hopoPage.showOnTabStaves.value = !hopoPage.showOnTabStaves.value
                    }
                }
            }
        }

        StyledGroupBox {
            Layout.fillWidth: true
            Layout.minimumWidth: 500
            title: qsTrc("notation/editstyle/hammeronpulloff", "Case")

            Row {
                width: parent.width
                spacing: 8

                RadioButtonGroup {
                    orientation: ListView.Horizontal

                    model: [
                        {iconCode: IconCode.HP_UPPER_CASE, toolTipTitle: qsTrc("notation/editstyle/hammeronpulloff", "Upper case"), value: true },
                        {iconCode: IconCode.HP_LOWER_CASE, toolTipTitle: qsTrc("notation/editstyle/hammeronpulloff", "Lower case"), value: false },
                    ]

                    delegate: FlatRadioButton {
                        required toolTipTitle
                        required iconCode
                        required property bool value

                        width: 40
                        height: 30

                        checked: hopoPage.hopoUpperCase.value === value
                        onToggled: hopoPage.hopoUpperCase.value = value
                    }
                }

                SeparatorLine {}

                FlatButton {
                    text: qsTrc("notation/editstyle/hammeronpulloff", "Edit text style")

                    onClicked: {
                        root.goToTextStylePage("hammer-ons-pull-offs-and-tapping")
                    }
                }
            }
        }

        StyledGroupBox {
            Layout.fillWidth: true
            Layout.minimumWidth: 500
            title: qsTrc("notation/editstyle/hammeronpulloff", "Alignment")

            ColumnLayout {
                width: parent.width
                spacing: 10

                StyledTextLabel {
                    text: qsTrc("notation/editstyle/hammeronpulloff", "Align ‘H’, ‘P’ and ‘T’ symbols in the same slur on")
                }

                CheckBox {
                    text: qsTrc("notation/editstyle/hammeronpulloff", "Standard staves")
                    checked: hopoPage.hopoAlignLettersStandardStaves.value === true
                    onClicked: hopoPage.hopoAlignLettersStandardStaves.value = !hopoPage.hopoAlignLettersStandardStaves.value
                }

                CheckBox {
                    text: qsTrc("notation/editstyle/hammeronpulloff", "Tablature staves")
                    checked: hopoPage.hopoAlignLettersTabStaves.value === true
                    onClicked: hopoPage.hopoAlignLettersTabStaves.value = !hopoPage.hopoAlignLettersTabStaves.value
                }
            }
        }

        StyledGroupBox {
            Layout.fillWidth: true
            Layout.minimumWidth: 500
            title: qsTrc("notation/editstyle/hammeronpulloff", "Consecutive hammer-ons/pull-offs")

            ColumnLayout {
                width: parent.width
                spacing: 8

                RadioButtonGroup {
                    orientation: ListView.Vertical
                    spacing: 8

                    model: [
                        {text: qsTrc("notation/editstyle/hammeronpulloff", "Show ‘H’ or ‘P’ between each pair of notes"), value: true },
                        {text: qsTrc("notation/editstyle/hammeronpulloff", "Show ‘H’ or ‘P’ once per group of ascending/descending notes"), value: false },
                    ]

                    delegate: RoundedRadioButton {
                        required text
                        required property bool value

                        checked: hopoPage.hopoShowAll.value === value
                        onToggled: hopoPage.hopoShowAll.value = value
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
                spacing: 8

                RowLayout {
                    spacing: 24

                    ColumnLayout {
                        spacing: 8

                        StyledTextLabel {
                            text: qsTrc("notation/editstyle/hammeronpulloff", "Show on standard staves")
                        }

                        RadioButtonGroup {
                            orientation: ListView.Horizontal

                            model: [
                                {text: qsTrc("notation/editstyle/hammeronpulloff", "Half slur"), value: 0 },
                                {text: qsTrc("notation/editstyle/hammeronpulloff", "Symbol"), value: 1 },
                                {text: qsTrc("notation/editstyle/hammeronpulloff", "Both"), value: 2 },
                            ]

                            delegate: FlatRadioButton {
                                required text
                                required property int value

                                width: Math.max(90, implicitContentWidth)
                                height: 30

                                checked: hopoPage.lhTappingShowItemsNormalStave.value === value
                                onToggled: hopoPage.lhTappingShowItemsNormalStave.value = value
                            }
                        }
                    }

                    ColumnLayout {
                        enabled: hopoPage.lhTappingShowItemsNormalStave.value !== 0
                        spacing: 8

                        StyledTextLabel {
                            text: qsTrc("notation/editstyle/hammeronpulloff", "Symbol")
                        }

                        RadioButtonGroup {
                            orientation: ListView.Horizontal

                            model: [
                                {iconCode: IconCode.TAPPING_ENCIRCLED_T, value: 1 },
                                {iconCode: IconCode.TAPPING_DOT, value: 0 },
                            ]

                            delegate: FlatRadioButton {
                                required iconCode
                                required property int value

                                width: 40
                                height: 30

                                checked: hopoPage.lhTappingSymbolNormalStave.value === value
                                onToggled: hopoPage.lhTappingSymbolNormalStave.value = value
                            }
                        }
                    }
                }

                RowLayout {
                    spacing: 24

                    ColumnLayout {
                        spacing: 8

                        StyledTextLabel {
                            text: qsTrc("notation/editstyle/hammeronpulloff", "Show on tablature staves")
                        }

                        RadioButtonGroup {
                            orientation: ListView.Horizontal

                            model: [
                                {text: qsTrc("notation/editstyle/hammeronpulloff", "Half slur"), value: 0 },
                                {text: qsTrc("notation/editstyle/hammeronpulloff", "Symbol"), value: 1 },
                                {text: qsTrc("notation/editstyle/hammeronpulloff", "Both"), value: 2 },
                            ]

                            delegate: FlatRadioButton {
                                required text
                                required property int value

                                width: Math.max(90, implicitContentWidth)
                                height: 30

                                checked: hopoPage.lhTappingShowItemsTab.value === value
                                onToggled: hopoPage.lhTappingShowItemsTab.value = value
                            }
                        }
                    }

                    ColumnLayout {
                        enabled: hopoPage.lhTappingShowItemsTab.value !== 0
                        spacing: 8

                        StyledTextLabel {
                            text: qsTrc("notation/editstyle/hammeronpulloff", "Symbol")
                        }

                        RadioButtonGroup {
                            orientation: ListView.Horizontal

                            model: [
                                {iconCode: IconCode.TAPPING_ENCIRCLED_T, value: 1 },
                                {iconCode: IconCode.TAPPING_DOT, value: 0 },
                            ]

                            delegate: FlatRadioButton {
                                required iconCode
                                required property int value

                                width: 40
                                height: 30

                                checked: hopoPage.lhTappingSymbolTab.value === value
                                onToggled: hopoPage.lhTappingSymbolTab.value = value
                            }
                        }
                    }
                }

                CheckBox {
                    enabled: hopoPage.lhTappingShowItemsTab.value !== 1
                    text: qsTrc("notation/editstyle/hammeronpulloff", "Slur top and bottom notes in chords")
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
                spacing: 12

                ColumnLayout {
                    spacing: 8

                    StyledTextLabel {
                        text: qsTrc("notation/editstyle/hammeronpulloff", "Symbol (standard staff)")
                    }

                    RadioButtonGroup {
                        orientation: ListView.Horizontal

                        model: [
                            {iconCode: IconCode.TAPPING_PLUS, value: 1},
                            {iconCode: IconCode.TAPPING_T, value: 0},
                        ]

                        delegate: FlatRadioButton {
                            required iconCode
                            required property int value

                            height: 30
                            width: 40

                            checked: hopoPage.rhTappingSymbolNormalStave.value === value
                            onToggled: hopoPage.rhTappingSymbolNormalStave.value = value
                        }
                    }
                }

                ColumnLayout {
                    spacing: 8

                    StyledTextLabel {
                        text: qsTrc("notation/editstyle/hammeronpulloff", "Symbol (tablature staff)")
                    }

                    RadioButtonGroup {
                        orientation: ListView.Horizontal

                        model: [
                            {iconCode: IconCode.TAPPING_PLUS, value: 1},
                            {iconCode: IconCode.TAPPING_T, value: 0},
                        ]

                        delegate: FlatRadioButton {
                            required iconCode
                            required property int value

                            height: 30
                            width: 40

                            checked: hopoPage.rhTappingSymbolTab.value === value
                            onToggled: hopoPage.rhTappingSymbolTab.value = value
                        }
                    }
                }
            }
        }
    }
}
