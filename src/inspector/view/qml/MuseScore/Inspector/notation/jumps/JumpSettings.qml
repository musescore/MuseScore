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
import QtQuick.Controls 2.15

import MuseScore.Ui 1.0
import MuseScore.Inspector 1.0
import MuseScore.UiComponents 1.0

import "../../common"

Column {
    id: root

    property QtObject model: null

    property NavigationPanel navigationPanel: null
    property int navigationRowOffset: 1

    objectName: "JumpSettings"

    spacing: 12

    TextSection {
        id: jumpTo
        titleText: qsTrc("inspector", "Jump to")
        propertyItem: root.model ? root.model.jumpTo : null

        navigation.panel: root.navigationPanel
        navigationRowStart: root.navigationRowOffset
    }

    TextSection {
        id: playUntil
        titleText: qsTrc("inspector", "Play until")
        propertyItem: root.model ? root.model.playUntil : null

        navigation.panel: root.navigationPanel
        navigationRowStart: jumpTo.navigationRowEnd + 1
    }

    TextSection {
        id: continueAt
        titleText: qsTrc("inspector", "Continue at")
        propertyItem: root.model ? root.model.continueAt : null

        navigation.panel: root.navigationPanel
        navigationRowStart: playUntil.navigationRowEnd + 1
    }

    CheckBox {
        isIndeterminate: root.model ? root.model.hasToPlayRepeats.isUndefined : false
        checked: root.model && !isIndeterminate ? root.model.hasToPlayRepeats.value : false
        text: qsTrc("inspector", "Play repeats")

        navigation.name: "PlayRepeats"
        navigation.panel: root.navigationPanel
        navigation.row: continueAt.navigationRowEnd + 1

        onClicked: { root.model.hasToPlayRepeats.value = !checked }
    }
}
