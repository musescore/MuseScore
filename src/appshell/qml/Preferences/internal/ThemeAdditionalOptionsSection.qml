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

import Muse.UiComponents 1.0

BaseSection {
    id: root

    property int scoreInversionMode: 0

    signal resetThemeToDefaultRequested()
    signal scoreInversionModeChangeRequested(int mode)

    ComboBoxWithTitle {
        id: scoreInversionModeDropdown
        width: parent.width

        title: qsTrc("appshell/preferences", "Invert score:")

        navigation.name: "ScoreInversionModeDropdown"
        navigation.panel: root.navigation
        navigation.row: 0

        model: [
            { text: qsTrc("appshell/preferences", "Disabled"), value: 0 },
            { text: qsTrc("appshell/preferences", "Follow app theme"), value: 1 },
            { text: qsTrc("appshell/preferences", "Always"), value: 2 },
        ]

        currentIndex: scoreInversionModeDropdown.indexOfValue(root.scoreInversionMode)

        onValueEdited: function(newIndex, newValue) {
            root.scoreInversionModeChangeRequested(newValue)
        }
    }

    FlatButton {
        text: qsTrc("appshell/preferences", "Reset to default")

        navigation.name: "ResetButton"
        navigation.panel: root.navigation
        navigation.row: 1

        onClicked: {
            root.resetThemeToDefaultRequested()
        }
    }
}
