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

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Project 1.0

StyledDialogView {
    id: root

    contentHeight: 300
    contentWidth: 500
    margins: 20

    objectName: "AudioGenerationSettingsDialog"

    ColumnLayout {
        id: content

        anchors.fill: parent

        spacing: 0

        StyledTextLabel {
            text: qsTrc("project/save", "Generate MP3 audio for web playback?")
            font: ui.theme.largeBodyBoldFont
        }

        StyledTextLabel {
            Layout.fillWidth: true
            Layout.topMargin: 15

            text: qsTrc("project/save", "This could take a few minutes each time you save, depending on the size of your score. These settings can always be changed in Preferences.")
            wrapMode: Text.WordWrap
            verticalAlignment: Text.AlignLeft
            horizontalAlignment: Text.AlignLeft
        }

        AudioGenerationSettings {
            Layout.fillWidth: true
            Layout.topMargin: 15
        }

        SeparatorLine {
            Layout.topMargin: 30
            Layout.leftMargin: -root.margins
            Layout.rightMargin: -root.margins
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.topMargin: 20

            CheckBox {
                id: dontAskAgain

                text: qsTrc("global", "Don't ask again")

                onClicked: {
                    checked = !checked
                }
            }

            Item {
                Layout.fillWidth: true
            }

            FlatButton {
                accentButton: true
                text: qsTrc("global", "OK")

                onClicked: {
                    root.ret = { "errcode": 0, "value": { "askAgain": !dontAskAgain.checked }}
                    root.hide()
                }
            }
        }
    }
}
