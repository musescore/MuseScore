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

import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0

FlatButton {
    id: root

    property bool checked: false

    signal toggleConcertPitchRequested()

    orientation: Qt.Horizontal
    transparent: true
    margins: 4

    contentItem: Row {
        spacing: 6

        width: implicitWidth + 4 // some extra right padding

        CheckBox {
            checked: root.checked

            onClicked: {
                root.toggleConcertPitchRequested()
            }
        }

        StyledIconLabel {
            anchors.verticalCenter: parent.verticalCenter

            iconCode: root.icon
            font: ui.theme.toolbarIconsFont
        }

        StyledTextLabel {
            anchors.verticalCenter: parent.verticalCenter

            text: root.text
        }
    }

    onClicked: {
        root.toggleConcertPitchRequested()
    }
}
