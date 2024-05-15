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

import MuseScore.NotationScene 1.0
import Muse.UiComponents 1.0
import Muse.Ui 1.0

Rectangle {
    anchors.fill: parent
    color: ui.theme.backgroundPrimaryColor

    AccidentalGroupPageModel {
        id: accidentalGroupPageModel
    }

    ColumnLayout {
        spacing: 12

        StyleToggleWithImage {
            id: followNoteOffset
            styleItem: accidentalGroupPageModel.accidFollowNoteOffset
            imageON: "accidentalImages/followOffset-ON.png"
            imageOFF: "accidentalImages/followOffset-OFF.png"
            text: qsTrc("notation", "Keep accidentals close to offset notes")
        }

        StyleToggleWithImage {
            id: keepSecondsTogether
            styleItem: accidentalGroupPageModel.keepAccidentalSecondsTogether
            imageON: "accidentalImages/secondsAdjacent-ON.png"
            imageOFF: "accidentalImages/secondsAdjacent-OFF.png"
            text: qsTrc("notation", "Pair accidentals on seconds")
        }

        StyleToggleWithImage {
            id: alignOctavesAcrossSubChords
            styleItem: accidentalGroupPageModel.alignAccidentalOctavesAcrossSubChords
            imageON: "accidentalImages/octaveSubchord-ON.png"
            imageOFF: "accidentalImages/octaveSubchord-OFF.png"
            text: qsTrc("notation", "Align accidentals an octave apart across subchords")
        }

        StyleToggleWithImage {
            id: alignOffsetOctaveAccidentals
            styleItem: accidentalGroupPageModel.alignOffsetOctaveAccidentals
            imageON: "accidentalImages/octaveOffset-ON.png"
            imageOFF: "accidentalImages/octaveOffset-OFF.png"
            text: qsTrc("notation", "Align accidentals on offset octaves")
        }
    }
}
