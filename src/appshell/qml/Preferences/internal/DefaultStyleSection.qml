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
import QtQuick.Layouts 1.15

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import MuseScore.Preferences 1.0

BaseSection {
    id: root

    title: qsTrc("appshell/preferences", "Default style")

    navigation.direction: NavigationPanel.Both

    property ScorePreferencesModel model: null

    GridLayout {
        width: parent.width
        columns: 2
        columnSpacing: root.columnSpacing
        rowSpacing: 4

        StyledTextLabel {
            id: styleLabel
            Layout.preferredWidth: root.columnWidth
            text: qsTrc("appshell/preferences/score", "Style for full score")
            horizontalAlignment: Text.AlignLeft
        }

        FilePicker {
            Layout.fillWidth: true

            dialogTitle: qsTrc("appshell/preferences/score", "Choose default style for full score")
            filter: qsTrc("appshell/preferences/score", "MuseScore style file") + " (*.mss)"
            dir: root.model ? root.model.defaultStylePath : ""
            path: root.model ? root.model.defaultStylePath : ""

            navigation: root.navigation
            navigationRowOrderStart: 0
            pathFieldTitle: styleLabel.text

            onPathEdited: function(newPath) {
                if (root.model) {
                    root.model.defaultStylePath = newPath
                }
            }
        }

        StyledTextLabel {
            id: partStyleLabel
            Layout.preferredWidth: root.columnWidth
            text: qsTrc("appshell/preferences", "Style for parts")
            horizontalAlignment: Text.AlignLeft
        }

        FilePicker {
            Layout.fillWidth: true

            dialogTitle: qsTrc("appshell/preferences", "Choose default style for parts")
            filter: qsTrc("appshell/preferences", "MuseScore style file") + " (*.mss)"
            dir: root.model ? root.model.defaultPartStylePath : ""
            path: root.model ? root.model.defaultPartStylePath : ""

            navigation: root.navigation
            navigationRowOrderStart: 1
            pathFieldTitle: partStyleLabel.text

            onPathEdited: function(newPath) {
                if (root.model) {
                    root.model.defaultPartStylePath = newPath
                }
            }
        }
    }
}
