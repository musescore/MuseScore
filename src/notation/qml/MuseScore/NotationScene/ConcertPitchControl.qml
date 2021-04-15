/**
 * // SPDX-License-Identifier: GPL-3.0-only
 * // MuseScore-CLA-applies
 * //=============================================================================
 * //  MuseScore
 * //  Music Composition & Notation
 * //
 * //  Copyright (C) 2021 MuseScore BVBA and others
 * //
 * //  This program is free software: you can redistribute it and/or modify
 * //  it under the terms of the GNU General Public License version 3 as
 * //  published by the Free Software Foundation.
 * //
 * //  This program is distributed in the hope that it will be useful,
 * //  but WITHOUT ANY WARRANTY; without even the implied warranty of
 * //  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * //  GNU General Public License for more details.
 * //
 * //  You should have received a copy of the GNU General Public License
 * //  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * //============================================================================= **/
import QtQuick 2.7

import MuseScore.NotationScene 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0

Row {
    spacing: 4

    ConcertPitchControlModel {
        id: model

        function toggleConcertPitch() {
            model.concertPitchEnabled = !model.concertPitchEnabled
        }
    }

    Component.onCompleted: {
        model.load()
    }

    CheckBox {
        anchors.verticalCenter: parent.verticalCenter

        checked: model.concertPitchEnabled

        onClicked: {
            model.toggleConcertPitch()
        }
    }

    FlatButton {
        icon: IconCode.TUNING_FORK
        text: qsTrc("notation", "Concert pitch")

        orientation: Qt.Horizontal
        normalStateColor: "transparent"

        onClicked: {
            model.toggleConcertPitch()
        }
    }
}
