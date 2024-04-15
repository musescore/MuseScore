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
import MuseScore.NotationScene 1.0

RowLayout {
    id: root

    spacing: 12

    property bool canOpenSelectedParts: false

    signal closeRequested()
    signal openSelectedPartsRequested()
    signal openAllPartsRequested()

    property NavigationPanel navigationPanel: NavigationPanel {
        name: "PartsBottomPanel"
        enabled: root.enabled && root.visible
        direction: NavigationPanel.Horizontal
        onActiveChanged: function(active) {
            if (active) {
                root.forceActiveFocus()
            }
        }
    }

    FlatButton {
        text: qsTrc("global", "Close")

        navigation.name: "CloseButton"
        navigation.panel: root.navigationPanel
        navigation.column: 0

        onClicked: {
            root.closeRequested()
        }
    }

    Item { Layout.fillWidth: true }

    FlatButton {
        text: qsTrc("global", "Open all")

        navigation.name: "OpenAllButton"
        navigation.panel: root.navigationPanel
        navigation.column: 1

        onClicked: {
            root.openAllPartsRequested()
        }
    }

    FlatButton {
        text: qsTrc("global", "Open selected")

        enabled: root.canOpenSelectedParts
        accentButton: true

        navigation.name: "OpenSelectedButton"
        navigation.panel: root.navigationPanel
        navigation.column: 2

        onClicked: {
            root.openSelectedPartsRequested()
        }
    }
}
