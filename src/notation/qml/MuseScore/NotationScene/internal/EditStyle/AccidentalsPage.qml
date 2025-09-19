/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
import QtQuick
import QtQuick.Layouts

import MuseScore.NotationScene
import Muse.UiComponents
import Muse.Ui

StyledFlickable {
    id: root

    contentWidth: contentLayout.implicitWidth
    contentHeight: contentLayout.implicitHeight

    AccidentalsPageModel {
        id: accidentalsPageModel
    }

    ColumnLayout {
        id: contentLayout
        anchors.fill: parent
        spacing: 12

        StyledGroupBox {
            Layout.fillWidth: true
            title: qsTrc("notation/editstyle/accidentals", "Accidentals")

            StyleSpinboxWithReset {
                styleItem: accidentalsPageModel.bracketedAccidentalPadding
                label: qsTrc("notation/editstyle/accidentals", "Padding inside parentheses:")

                suffix: qsTrc("global", "sp")
                decimals: 3
                step: 0.1
                min: -10.0
                max: 10.0

                labelAreaWidth: -1
                controlAreaWidth: spinBoxWidth
            }
        }

        StyledGroupBox {
            Layout.fillWidth: true
            title: qsTrc("notation/editstyle/accidentals", "♮ in key signature changes")

            RadioButtonGroup {
                width: parent.width
                orientation: ListView.Vertical
                spacing: 8

                model: [
                    { text: qsTrc("notation/editstyle/accidentals", "Only for a change to C major / A minor"), value: /* KeySigNatural::NONE */ 0 },
                    { text: qsTrc("notation/editstyle/accidentals", "Before key signature if changing to fewer ♯ or ♭"), value: /* KeySigNatural::BEFORE */ 1 },
                    { text: qsTrc("notation/editstyle/accidentals", "After key signature if changing to fewer ♯ or ♭; before if changing between ♯ and ♭"), value: /* KeySigNatural::AFTER */ 2 }
                ]

                delegate: RoundedRadioButton {
                    width: ListView.view.width
                    text: modelData.text
                    checked: accidentalsPageModel.keySigNaturals.value === modelData.value
                    onToggled: accidentalsPageModel.keySigNaturals.value = modelData.value
                }
            }
        }

        StyledGroupBox {
            Layout.fillWidth: true
            title: qsTrc("notation/editstyle/accidentals", "Multiple accidentals in chords")

            ColumnLayout {
                spacing: 12

                StyleToggleWithImage {
                    id: followNoteOffset
                    styleItem: accidentalsPageModel.accidFollowNoteOffset
                    imageON: "accidentalImages/followOffset-ON.png"
                    imageOFF: "accidentalImages/followOffset-OFF.png"
                    text: qsTrc("notation", "Keep accidentals close to offset notes")
                }

                StyleToggleWithImage {
                    id: keepSecondsTogether
                    styleItem: accidentalsPageModel.keepAccidentalSecondsTogether
                    imageON: "accidentalImages/secondsAdjacent-ON.png"
                    imageOFF: "accidentalImages/secondsAdjacent-OFF.png"
                    text: qsTrc("notation", "Pair accidentals on seconds")
                }

                StyleToggleWithImage {
                    id: alignOctavesAcrossSubChords
                    styleItem: accidentalsPageModel.alignAccidentalOctavesAcrossSubChords
                    imageON: "accidentalImages/octaveSubchord-ON.png"
                    imageOFF: "accidentalImages/octaveSubchord-OFF.png"
                    text: qsTrc("notation", "Align accidentals an octave apart across subchords")
                }

                StyleToggleWithImage {
                    id: alignOffsetOctaveAccidentals
                    styleItem: accidentalsPageModel.alignOffsetOctaveAccidentals
                    imageON: "accidentalImages/octaveOffset-ON.png"
                    imageOFF: "accidentalImages/octaveOffset-OFF.png"
                    text: qsTrc("notation", "Align accidentals on offset octaves")
                }
            }
        }
    }
}
