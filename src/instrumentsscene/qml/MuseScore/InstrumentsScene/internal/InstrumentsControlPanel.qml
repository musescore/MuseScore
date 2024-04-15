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

RowLayout {
    id: root

    property bool isMovingUpAvailable: false
    property bool isMovingDownAvailable: false
    property bool isRemovingAvailable: false
    property bool isAddingAvailable: value
    property bool isInstrumentSelected: false

    property alias navigation: keynavSub

    signal addRequested()
    signal moveUpRequested()
    signal moveDownRequested()
    signal removingRequested()

    spacing: 6

    focus: true

    Keys.onShortcutOverride: function(event) {
        if (event.key === Qt.Key_Delete) {
            root.removingRequested()
        }
    }

    NavigationPanel {
        id: keynavSub
        name: "InstrumentsHeader"
        enabled: root.enabled && root.visible
    }

    FlatButton {
        Layout.fillWidth: true

        navigation.name: "Add"
        navigation.panel: keynavSub
        navigation.order: 1
        accessible.name: qsTrc("instruments", "Add instruments")

        //: Keep in sync with the message that appears if there are no instruments in the score (InstrumentsPanel.qml)
        text: qsTrc("instruments", "Add")

        enabled: root.isAddingAvailable

        onClicked: {
            root.addRequested()
        }
    }

    FlatButton {
        Layout.preferredWidth: width

        navigation.name: "Up"
        navigation.panel: keynavSub
        navigation.order: 2

        toolTipTitle: root.isInstrumentSelected ? qsTrc("instruments", "Move selected instruments up") : qsTrc("instruments", "Move selected staves up")

        enabled: root.isMovingUpAvailable

        icon: IconCode.ARROW_UP

        onClicked: {
            root.moveUpRequested()
        }
    }

    FlatButton {
        Layout.preferredWidth: width

        navigation.name: "Down"
        navigation.panel: keynavSub
        navigation.order: 3

        toolTipTitle: root.isInstrumentSelected ? qsTrc("instruments", "Move selected instruments down") : qsTrc("instruments", "Move selected staves down")

        enabled: root.isMovingDownAvailable

        icon: IconCode.ARROW_DOWN

        onClicked: {
            root.moveDownRequested()
        }
    }

    FlatButton {
        Layout.preferredWidth: width

        navigation.name: "Remove"
        navigation.panel: keynavSub
        navigation.order: 4

        toolTipTitle: root.isInstrumentSelected ? qsTrc("instruments", "Remove selected instruments") : qsTrc("instruments", "Remove selected staves")

        enabled: root.isRemovingAvailable

        icon: IconCode.DELETE_TANK

        onClicked: {
            root.removingRequested()
        }
    }
}
