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

import MuseScore.Ui 1.0
import MuseScore.Dock 1.0
import MuseScore.UiComponents 1.0

DockPage {
    id: sequencerPage

    objectName: "Sequencer"

    property var color: ui.theme.backgroundPrimaryColor

    toolbar: DockToolBar {

        id: seqToolBar
        objectName: "seqToolBar"

        minimumWidth: 400
        minimumHeight: 40
        color: sequencerPage.color

        Rectangle {
            color: sequencerPage.color

            StyledTextLabel {
                anchors.fill: parent
                text: "Sequencer toolbar"
            }
        }
    }

    panels: [
        DockPanel {

            id: mixerPanel
            objectName: "mixerPanel"

            title: "Mixer"
            width: 200

            Rectangle {

                StyledTextLabel {
                    anchors.fill: parent
                    text: "Mixer"
                }
            }
        }
    ]

    central: DockCentral {

        id: seqCentral
        objectName: "seqCentral"

        Rectangle {
            StyledTextLabel {
                anchors.fill: parent
                text: "Sequencer"
            }
        }
    }

    statusbar: DockStatusBar {

        id: notationStatusBar
        objectName: "seqStatusBar"

        width: 400
        color: sequencerPage.color

        Rectangle {
            color: notationStatusBar.color

            StyledTextLabel {
                anchors.fill: parent
                text: "Sequencer status bar"
            }
        }
    }
}
