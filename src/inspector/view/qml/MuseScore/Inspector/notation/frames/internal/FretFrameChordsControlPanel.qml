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

import MuseScore.Inspector 1.0

import "../../../common"

RowLayout {
    id: root

    property PropertyItem listOrderItem: null

    property NavigationPanel navigationPanel: null
    property int navigationRowStart: 0
    property int navigationRowEnd: resetButton.navigation.row

    property bool isMovingUpAvailable: false
    property bool isMovingDownAvailable: false
    property bool hasInvisibleChords: false

    height: childrenRect.height

    spacing: 6

    signal moveSelectionUpRequested()
    signal moveSelectionDownRequested()
    signal resetListRequested();

    FlatButton {
        id: upButton

        Layout.fillWidth: true

        icon: IconCode.ARROW_UP
        enabled: root.isMovingUpAvailable
        toolTipTitle: qsTrc("inspector", "Move up")

        navigation.name: "MoveUpButton"
        navigation.panel: root.navigationPanel
        navigation.row: root.navigationRowStart

        onClicked: {
            root.moveSelectionUpRequested()
        }
    }

    FlatButton {
        id: downButton

        Layout.fillWidth: true

        icon: IconCode.ARROW_DOWN
        enabled: root.isMovingDownAvailable
        toolTipTitle: qsTrc("inspector", "Move down")

        navigation.name: "MoveDownButton"
        navigation.panel: root.navigationPanel
        navigation.row: upButton.navigation.row + 1

        onClicked: {
            root.moveSelectionDownRequested()
        }
    }

    PropertyResetButton {
        id: resetButton

        enabled: root.listOrderItem.isModified || root.hasInvisibleChords
        toolTipTitle: qsTrc("inspector", "Reset chord list")

        navigation.name: "ResetButton"
        navigation.panel: root.navigationPanel
        navigation.row: downButton.navigation.row + 1

        onClicked: {
            root.resetListRequested()
        }
    }
}
