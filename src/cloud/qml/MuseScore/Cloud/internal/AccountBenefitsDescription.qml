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
import MuseScore.UiComponents 1.0

Column {
    spacing: 60

    Column {
        spacing: 32

        StyledTextLabel {
            text: qsTrc("cloud", "What are the benefits of a MuseScore account?")
            font: ui.theme.headerBoldFont
        }

        StyledTextLabel {
            text: qsTrc("cloud", "A MuseScore profile allows you to save & publish your scores on MuseScore.com. It's free.")
            font: ui.theme.largeBodyFont
        }
    }

    ListView {
        height: contentHeight
        width: contentWidth

        spacing: 32

        model: [
            qsTrc("cloud", "Save your scores to private cloud area"),
            qsTrc("cloud", "Share links with other musicians, who add comments"),
            qsTrc("cloud", "Create a portfolio for your music and gain followers"),
            qsTrc("cloud", "Upload high quality audio for superb score playback")
        ]

        delegate: Row {
            spacing: 38

            Rectangle {
                width: 9
                height: width
                color: ui.theme.accentColor
                radius: width / 2
                anchors.verticalCenter: parent.verticalCenter
            }

            StyledTextLabel {
                text: modelData
                font: ui.theme.largeBodyFont
            }
        }
    }
}
