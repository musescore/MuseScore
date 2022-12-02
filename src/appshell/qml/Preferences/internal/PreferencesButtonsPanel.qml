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

Rectangle {
    id: root

    color: ui.theme.backgroundPrimaryColor

    property NavigationPanel navigation: NavigationPanel {
        name: "PreferencesButtonsPanel"
        enabled: root.enabled && root.visible
        direction: NavigationPanel.Horizontal

        onActiveChanged: function(active) {
            if (active) {
                root.forceActiveFocus()
            }
        }
    }

    signal revertFactorySettingsRequested()
    signal applyRequested()
    signal rejectRequested()

    Item {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        anchors.margins: 20

        height: childrenRect.height

        FlatButton {
            anchors.left: parent.left

            minWidth: 160

            navigation.panel: root.navigation
            navigation.order: 2

            text: qsTrc("appshell/preferences", "Reset preferences")

            onClicked: {
                root.revertFactorySettingsRequested()
            }
        }

        FlatButton {
            anchors.right: applyButton.left
            anchors.rightMargin: 12

            navigation.panel: root.navigation
            navigation.order: 3

            text: qsTrc("global", "Cancel")

            onClicked: {
                root.rejectRequested()
            }
        }

        FlatButton {
            id: applyButton

            anchors.right: parent.right

            navigation.panel: root.navigation
            navigation.order: 1

            accentButton: true

            text: qsTrc("global", "OK")

            onClicked: {
                root.applyRequested()
            }
        }
    }
}
