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
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import MuseScore.UiComponents 1.0

Rectangle {
    color: ui.theme.backgroundPrimaryColor

    ColumnLayout {
        anchors.fill: parent

        Row {
            Layout.fillWidth: true

            FlatButton {
                text: "Ok"
                //onClicked
            }
            FlatButton {
                text: "Next"
                //onClicked
            }
        }

        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            ScrollBar.horizontal.interactive: true
            ScrollBar.vertical.interactive: true
            ScrollBar.horizontal.policy: ScrollBar.AlwaysOn
            ScrollBar.vertical.policy: ScrollBar.AlwaysOn

            Label {
                text: "ABC"
                font.pixelSize: 224
            }

            contentWidth: 1500
            contentHeight: 500

            PianorollView {

                implicitWidth: 1500
                implicitHeight: 500

                StyledTextLabel {
                    anchors.centerIn: parent
                    text: pianoRollPanel.title + " Walla"
                }

                Component.onCompleted: {
                    load()
                }
            }
        }
    }
}
