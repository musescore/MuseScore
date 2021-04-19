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

    FoldersPreferencesModel {
        id: foldersPreferencesModel
    }

    Component.onCompleted: {
        foldersPreferencesModel.load()
    }

    Column {
        id: content

        anchors.left: parent.left
        anchors.right: parent.right
        spacing: 18

        StyledTextLabel {
            text: qsTrc("appshell", "Folders")
            font: ui.theme.bodyBoldFont
        }

        ListView {
            anchors.left: parent.left
            anchors.right: parent.right

            height: contentHeight

            spacing: 4

            model: foldersPreferencesModel

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

                    pickerType: FilePicker.PickerType.Directory
                    dialogTitle: qsTrc("appshell", "Choose %1 Folder").arg(model.title)
                    dir: model.path

                    path: model.path

                    onPathEdited: {
                        model.path = newPath
                    }
                }
            }
        }
    }
}
