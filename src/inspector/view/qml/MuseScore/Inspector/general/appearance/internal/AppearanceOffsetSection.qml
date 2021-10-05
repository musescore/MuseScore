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

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Inspector 1.0

import "../../../common"

Column {
    id: root

    property alias horizontalOffset: offsets.horizontalOffset
    property alias verticalOffset: offsets.verticalOffset

    property bool isSnappedToGrid: false

    property NavigationPanel navigationPanel: null
    property int navigationRowStart: 0

    signal snapToGridToggled(var snap)
    signal configureGridRequested()

    height: implicitHeight
    width: parent.width

    spacing: 12

    QtObject {
        id: prv

        function navigationRow(r) {
            return offsets.navigationRowEnd + r
        }
    }

    OffsetSection {
        id: offsets

        navigationPanel: root.navigationPanel
        navigationRowStart: root.navigationRowStart
    }

    CheckBox {
        id: snapToGridCheckbox

        navigation.name: "Snap to grid"
        navigation.panel: root.navigationPanel
        navigation.row: prv.navigationRow(1)

        text: qsTrc("inspector", "Snap to grid")

        checked: isSnappedToGrid

        onClicked: {
            root.snapToGridToggled(!checked)
        }
    }

    FlatButton {
        width: parent.width

        navigation.name: "Configure grid"
        navigation.panel: root.navigationPanel
        navigation.row: prv.navigationRow(2)

        text: qsTrc("inspector", "Configure grid")

        visible: snapToGridCheckbox.checked

        onClicked: {
            root.configureGridRequested()
        }
    }
}
