/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

import QtQuick
import QtQuick.Layouts

import Muse.Ui
import Muse.UiComponents
import Muse.Shortcuts

RowLayout {
    id: root

    property alias canResetCurrentShortcut: resetButton.enabled

    property int buttonMinWidth: 0

    signal importShortcutsFromFileRequested()
    signal exportShortcutsToFileRequested()
    signal resetToDefaultSelectedShortcuts()

    property NavigationPanel navigation: NavigationPanel {
        name: "ShortcutsBottomPanel"
        enabled: root.enabled && root.visible
        direction: NavigationPanel.Horizontal
        accessible.name: qsTrc("shortcuts", "Shortcuts bottom panel")

        onActiveChanged: function(active) {
            if (active) {
                root.forceActiveFocus()
            }
        }
    }

    FlatButton {
        minWidth: root.buttonMinWidth

        text: qsTrc("shortcuts", "Import")

        navigation.name: "ImportButton"
        navigation.panel: root.navigation
        navigation.column: 0

        onClicked: {
            root.importShortcutsFromFileRequested()
        }
    }

    FlatButton {
        minWidth: root.buttonMinWidth

        text: qsTrc("shortcuts", "Export")

        navigation.name: "ExportButton"
        navigation.panel: root.navigation
        navigation.column: 1

        onClicked: {
            root.exportShortcutsToFileRequested()
        }
    }

    Item { Layout.fillWidth: true }

    FlatButton {
        id: resetButton

        minWidth: root.buttonMinWidth

        text: qsTrc("shortcuts", "Reset to default")

        navigation.name: "ResetButton"
        navigation.panel: root.navigation
        navigation.column: 2

        onClicked: {
            root.resetToDefaultSelectedShortcuts()
        }
    }
}
