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
import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.UserScores 1.0

FlatButton {
    id: root

    property var model: null

    height: 96
    accentButton: popup.visible

    KeySignature {
        icon: model.keySignature.icon
        text: model.keySignature.title
    }

    onClicked: {
        if (!popup.isOpened) {
            popup.open()
        } else {
            popup.close()
        }
    }

    StyledPopup {
        id: popup

        implicitHeight: 300
        implicitWidth: 724

        arrowX: root.x + root.width / 2
        y: root.height

        Column {
            id: column

            anchors.fill: parent
            anchors.topMargin: 10
            anchors.margins: 20

            spacing: 20

            TabBar {
                id: bar

                anchors.horizontalCenter: parent.horizontalCenter

                implicitHeight: 28
                spacing: 0

                StyledTabButton {
                    text: qsTrc("userscores", "Major")
                    sideMargin: 22
                    isCurrent: bar.currentIndex === 0
                }

                StyledTabButton {
                    text: qsTrc("appshell", "Minor")
                    sideMargin: 22
                    isCurrent: bar.currentIndex === 1
                }
            }

            StackLayout {
                id: pagesStack

                anchors.left: parent.left
                anchors.right: parent.right

                currentIndex: bar.currentIndex

                KeySignatureListView {
                    model: root.model.keySignatureMajorList()
                    currentSignature: root.model.keySignature

                    onSignatureSelected: {
                        root.model.keySignature = signature
                    }
                }

                KeySignatureListView {
                    model: root.model.keySignatureMinorList()
                    currentSignature: root.model.keySignature

                    onSignatureSelected: {
                        root.model.keySignature = signature
                    }
                }
            }
        }
    }
}
