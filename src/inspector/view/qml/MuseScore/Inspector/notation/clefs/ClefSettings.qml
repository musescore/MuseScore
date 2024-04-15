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

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import MuseScore.Inspector 1.0

import "../../common"

Column {
    id: root

    property QtObject model: null

    property NavigationPanel navigationPanel: null
    property int navigationRowStart: 1

    objectName: "ClefSettings"

    spacing: 12

    function focusOnFirst() {
        showCourtesyClef.navigation.requestActive()
    }

    PropertyCheckBox {
        id: showCourtesyClef

        navigation.name: "ShowCourtesyClefCheckBox"
        navigation.panel: root.navigationPanel
        navigation.row: root.navigationRowStart

        text: qsTrc("inspector", "Show courtesy clef on previous system")
        propertyItem: root.model ? root.model.shouldShowCourtesy : null
    }

    FlatRadioButtonGroupPropertyView {
        id: clefToBarlinePosition
        enabled: root.model ? root.model.isClefToBarPosAvailable : false

        titleText: qsTrc("inspector", "Position relative to barline")
        propertyItem: root.model ? root.model.clefToBarlinePosition : null

        navigationName: "ClefToBarlinePosition"
        navigationPanel: root.navigationPanel
        navigationRowStart: showCourtesyClef.navigation.row + 1

        model: [
            { text: qsTrc("inspector", "Auto"), value: 0, title: qsTrc("inspector", "Auto") },
            { text: qsTrc("inspector", "Before"), value: 1, title: qsTrc("inspector", "Before") },
            { text: qsTrc("inspector", "After"), value: 2, title: qsTrc("inspector", "After") }
        ]
    }
}
