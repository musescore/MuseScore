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

import Muse.Ui 1.0
import Muse.UiComponents 1.0

RowLayout {
    id: root

    property alias defaultButtonName: openMuseHubButton.text
    property alias cancelButtonName: cancelBtn.text

    property NavigationPanel navigationPanel: NavigationPanel {
        name: "UpdateBottomPanel"
        direction: NavigationPanel.Horizontal
    }

    spacing: 12

    signal skipRequested()
    signal openMuseHubRequested()

    function focusOnFirst() {
        openMuseHubButton.navigation.requestActive()
    }

    FlatButton {
        id: cancelBtn
        Layout.alignment: Qt.AlignVCenter
        Layout.preferredWidth: (root.width - root.spacing) / 2

        navigation.name: "NoButton"
        navigation.panel: root.navigationPanel
        navigation.column: 2

        onClicked: {
            root.skipRequested()
        }
    }

    FlatButton {
        id: openMuseHubButton

        Layout.alignment: Qt.AlignVCenter
        Layout.preferredWidth: (root.width - root.spacing) / 2

        accentButton: true

        navigation.name: "OpenMuseHubButton"
        navigation.panel: root.navigationPanel
        navigation.column: 1
        accessible.ignored: true
        navigation.onActiveChanged: {
            if (!navigation.active) {
                accessible.ignored = false
            }
        }

        onClicked: {
            root.openMuseHubRequested()
        }
    }
}
