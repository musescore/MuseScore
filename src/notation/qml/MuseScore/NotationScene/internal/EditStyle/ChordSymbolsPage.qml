/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore BVBA and others
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

    contentWidth: column.width
    contentHeight: column.height

    ChordSymbolsPageModel {
        id: chordSymbolsModel
    }

    signal goToTextStylePage(string s)

    ColumnLayout {
        id: column
        spacing: 12

        StyledGroupBox {
            Layout.fillWidth: true
            Layout.minimumWidth: 500
            title: qsTrc("notation/editstyle/chordsymbols", "Preset")
        }

        StyledGroupBox {
            Layout.fillWidth: true
            Layout.minimumWidth: 500
            title: qsTrc("notation/editstyle/chordsymbols", "Appearance")

            ColumnLayout {
                spacing: 12
                anchors.fill: parent

                FlatButton {
                    text: qsTrc("notation", "Edit chord symbol text style")

                    onClicked: {
                        root.goToTextStylePage("chord-symbols")
                    }
                }

                StyledGroupBox {
                    Layout.fillWidth: true
                    title: qsTrc("notation/editstyle/chordsymbols", "Spelling")
                }

                StyledGroupBox {
                    Layout.fillWidth: true
                    title: qsTrc("notation/editstyle/chordsymbols", "Automatically capitalize note names")

                    RowLayout {
                        spacing: 6
                        anchors.fill: parent

                        CheckBox {
                            text: qsTrc("notation/editstyle/chordsymbols", "Lowercase minor chords")
                            checked: chordSymbolsModel.lowerCaseMinorChords.value === true
                            onClicked: chordSymbolsModel.lowerCaseMinorChords.value = !chordSymbolsModel.lowerCaseMinorChords.value
                        }

                        CheckBox {
                            text: qsTrc("notation/editstyle/chordsymbols", "Lowercase bass notes")
                            checked: chordSymbolsModel.lowerCaseBassNotes.value === true
                            onClicked: chordSymbolsModel.lowerCaseBassNotes.value = !chordSymbolsModel.lowerCaseBassNotes.value
                        }

                        CheckBox {
                            text: qsTrc("notation/editstyle/chordsymbols", "All caps note names")
                            checked: chordSymbolsModel.allCapsNoteNames.value === true
                            onClicked: chordSymbolsModel.allCapsNoteNames.value = !chordSymbolsModel.allCapsNoteNames.value
                        }

                        //TODO RESET
                    }
                }

                StyledGroupBox {
                    Layout.fillWidth: true
                    title: qsTrc("notation/editstyle/chordsymbols", "Extensions (e.g. 7, 11)")
                    ColumnLayout {
                        spacing: 12
                        anchors.fill: parent

                        StyleSpinboxRow {
                            styleItem: chordSymbolsModel.extensionMag
                            label: qsTrc("notation", "Scale:")
                            inPercentage: true
                            hasReset: false
                        }
                        StyleSpinboxRow {
                            styleItem: chordSymbolsModel.extensionAdjust
                            label: qsTrc("notation", "Vertical offset:")
                            inPercentage: true
                            hasReset: false
                        }
                    }
                }

                StyledGroupBox {
                    Layout.fillWidth: true
                    title: qsTrc("notation/editstyle/chordsymbols", "Modifiers (e.g. sus4, no 3)")

                    ColumnLayout {
                        spacing: 12
                        anchors.fill: parent

                        StyleSpinboxRow {
                            styleItem: chordSymbolsModel.modifierMag
                            label: qsTrc("notation", "Scale:")
                            inPercentage: true
                            hasReset: false
                        }
                        StyleSpinboxRow {
                            styleItem: chordSymbolsModel.modifierAdjust
                            label: qsTrc("notation", "Vertical offset:")
                            inPercentage: true
                            hasReset: false
                        }
                    }
                }

                StyledGroupBox {
                    Layout.fillWidth: true
                    title: qsTrc("notation/editstyle/chordsymbols", "Altered bass notes (eg. A7/G)")
                }

                StyledGroupBox {
                    Layout.fillWidth: true
                    title: qsTrc("notation/editstyle/chordsymbols", "Polychords (e.g. C|F♯)")
                }
            }
        }

        StyledGroupBox {
            Layout.fillWidth: true
            Layout.minimumWidth: 500
            title: qsTrc("notation/editstyle/chordsymbols", "Alignment")
        }

        StyledGroupBox {
            Layout.fillWidth: true
            Layout.minimumWidth: 500
            title: qsTrc("notation/editstyle/chordsymbols", "Positioning")

            ColumnLayout {
                spacing: 12
                anchors.fill: parent

                StyleSpinboxRow {
                    styleItem: chordSymbolsModel.harmonyFretDist
                    label: qsTrc("notation", "Minimum space from fretboard diagram:")
                    suffix: qsTrc("global", "sp")
                    step: 0.1
                    hasReset: false
                }

                StyleSpinboxRow {
                    styleItem: chordSymbolsModel.minHarmonyDist
                    label: qsTrc("notation", "Minimum space between chord symbols:")
                    suffix: qsTrc("global", "sp")
                    step: 0.1
                    hasReset: false
                }
            }
        }

        StyledGroupBox {
            Layout.fillWidth: true
            Layout.minimumWidth: 500
            title: qsTrc("notation/editstyle/chordsymbols", "Playback")

            ColumnLayout {
                spacing: 12
                width: parent.width

                StyleDropdownRow {
                    styleItem: chordSymbolsModel.harmonyVoiceLiteral
                    label: qsTrc("notation", "Interpretation:")
                    hasReset: false
                    model: chordSymbolsModel.possibleHarmonyVoiceLiteralOptions()
                }

                StyleDropdownRow {
                    styleItem: chordSymbolsModel.harmonyVoicing
                    label: qsTrc("notation", "Voicing:")
                    hasReset: false
                    model: chordSymbolsModel.possibleHarmonyVoicingOptions()
                }

                StyleDropdownRow {
                    styleItem: chordSymbolsModel.harmonyDuration
                    label: qsTrc("notation", "Duration:")
                    hasReset: false
                    model: chordSymbolsModel.possibleHarmonyDurationOptions()
                }

                StyleSpinboxRow {
                    styleItem: chordSymbolsModel.capoPosition
                    label: qsTrc("notation", "Capo fret position:")
                    hasReset: false
                    step: 1
                }
            }
        }
    }
}
