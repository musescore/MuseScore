/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
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
import QtQuick.Controls

import Muse.Ui
import Muse.UiComponents

Row {
    id: root

    property alias colors: colorsList.colors
    property alias currentColorIndex: colorsList.currentColorIndex

    property NavigationPanel navigation: NavigationPanel {
        name: titleLabel.text
        enabled: root.enabled && root.visible
        direction: NavigationPanel.Both
        accessible.name: titleLabel.text

        onActiveChanged: function(active) {
            if (active) {
                root.forceActiveFocus()
            }
        }
    }

    property int columnWidth: 0

    signal accentColorChangeRequested(var newColorIndex)

    height: colorsList.height
    spacing: 12

    StyledTextLabel {
        id: titleLabel
        width: root.columnWidth

        anchors.top: parent.top
        height: colorsList.firstRowHeight
        horizontalAlignment: Qt.AlignLeft
        verticalAlignment: Qt.AlignVCenter

        text: qsTrc("preferences", "Accent color")
    }

    AccentColorsList {
        id: colorsList

        navigationPanel: root.navigation

        onAccentColorChangeRequested: function(newColorIndex) {
            root.accentColorChangeRequested(newColorIndex)
        }
    }
}
