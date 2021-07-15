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

import MuseScore.UiComponents 1.0
import MuseScore.Preferences 1.0

PreferencesPage {
    id: root

    contentHeight: content.height

    ScorePreferencesModel {
        id: scorePreferencesModel
    }

    Component.onCompleted: {
        scorePreferencesModel.load()
    }

    Column {
        anchors.fill: parent
        spacing: 24
        Column {
            id: content

            anchors.left: parent.left
            anchors.right: parent.right
            spacing: 18

            StyledTextLabel {
                text: qsTrc("appshell", "Default files")
                font: ui.theme.bodyBoldFont
            }

            ListView {
                anchors.left: parent.left
                anchors.right: parent.right

                height: contentHeight

                spacing: 4

                model: scorePreferencesModel

                delegate: RowLayout {
                    width: ListView.view.width
                    height: 30

                    spacing: 20

                    StyledTextLabel {
                        Layout.alignment: Qt.AlignLeft

                        text: model.title + ":"
                    }

                    FilePicker {
                        Layout.alignment: Qt.AlignRight
                        Layout.preferredWidth: 380

                        dialogTitle: model.chooseTitle
                        filter: model.pathFilter
                        dir: scorePreferencesModel.fileDirectory(model.path)

                        path: model.path

                        onPathEdited: {
                            model.path = newPath
                        }
                    }
                }
            }
        }

        SeparatorLine { }

        Column {
            anchors.left: parent.left
            anchors.right: parent.right
            spacing: 18

            StyledTextLabel {
                text: qsTrc("appshell", "View")
                font: ui.theme.bodyBoldFont
            }

            CheckBox {
                width: 216
                text: qsTrc("appshell", "Show MIDI controls in mixer")

                checked: scorePreferencesModel.isShowMIDIControls

                onClicked: {
                    scorePreferencesModel.isShowMIDIControls = !scorePreferencesModel.isShowMIDIControls
                }
            }
        }
    }
}
