/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import MuseScore.Preferences

BaseSection {
    id: root

    title: qsTrc("appshell/preferences", "Program start")

    navigation.direction: NavigationPanel.Both

    required property GeneralPreferencesModel model

    signal currentStartupModesChanged(int index)
    signal startupScorePathChanged(string path)

    rowSpacing: 16

    RadioButtonGroup {
        id: startupModesBox

        spacing: root.rowSpacing
        orientation: Qt.Vertical

        width: parent.width

        model: root.model.startupModes

        delegate: Row {
            width: parent.width
            height: radioButton.implicitHeight
            spacing: root.columnSpacing

            RoundedRadioButton {
                id: radioButton
                anchors.verticalCenter: parent.verticalCenter

                width: filePickerLoader.active ? Math.max(implicitWidth, root.columnWidth)
                                               : parent.width

                text: modelData.title
                checked: modelData.value === root.model.currentStartupMode

                navigation.name: modelData.title
                navigation.panel: root.navigation
                navigation.row: model.index
                navigation.column: 0

                onToggled: {
                    root.model.currentStartupMode = modelData.value
                }
            }

            Loader {
                id: filePickerLoader
                active: modelData.isStartWithScore ?? false
                anchors.verticalCenter: parent.verticalCenter

                sourceComponent: FilePicker {
                    enabled: radioButton.checked

                    pathFieldWidth: root.columnWidth
                    spacing: root.columnSpacing

                    dialogTitle: qsTrc("appshell/preferences", "Choose starting score")
                    filter: root.model.scorePathFilter

                    path: root.model.startupScorePath

                    navigation: root.navigation
                    navigationRowOrderStart: model.index
                    navigationColumnOrderStart: 1

                    onPathEdited: function(newPath) {
                        root.model.startupScorePath = newPath
                    }
                }
            }
        }
    }
}
