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
import QtQuick.Controls 2.15

import Muse.Ui 1.0
import MuseScore.Inspector 1.0
import Muse.UiComponents 1.0

import "../../common"

Column {
    id: root

    property QtObject model: null

    property NavigationPanel navigationPanel: null
    property int navigationRowStart: 1

    objectName: "JumpSettings"

    spacing: 12

    function focusOnFirst() {
        jumpTo.focusOnFirst()
    }

    TextSection {
        id: jumpTo
        titleText: qsTrc("inspector", "Jump to")
        propertyItem: root.model ? root.model.jumpTo : null

        navigationPanel: root.navigationPanel
        navigationRowStart: root.navigationRowStart
    }

    TextSection {
        id: playUntil
        titleText: qsTrc("inspector", "Play until")
        propertyItem: root.model ? root.model.playUntil : null

        navigationPanel: root.navigationPanel
        navigationRowStart: jumpTo.navigationRowEnd + 1
    }

    TextSection {
        id: continueAt
        titleText: qsTrc("inspector", "Continue at")
        propertyItem: root.model ? root.model.continueAt : null

        navigationPanel: root.navigationPanel
        navigationRowStart: playUntil.navigationRowEnd + 1
    }

    PropertyCheckBox {
        text: qsTrc("inspector", "Play repeats")
        propertyItem: root.model ? root.model.hasToPlayRepeats : null

        navigation.name: "PlayRepeats"
        navigation.panel: root.navigationPanel
        navigation.row: continueAt.navigationRowEnd + 1
    }
}
