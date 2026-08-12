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

import Muse.Ui
import Muse.UiComponents
import MuseScore.PropertiesPanel

import "../../common"

Column {
    id: root

    required property ClefSettingsModel model

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

        enabled: root.model ? root.model.isCourtesyClefAvailable : false

        text: qsTrc("propertiespanel", "Show courtesy clef")
        propertyItem: root.model ? root.model.shouldShowCourtesy : null
    }

    FlatRadioButtonGroupPropertyView {
        id: clefToBarlinePosition
        enabled: root.model ? root.model.isClefToBarPosAvailable : false

        titleText: qsTrc("propertiespanel", "Position relative to barline")
        propertyItem: root.model ? root.model.clefToBarlinePosition : null

        navigationName: "ClefToBarlinePosition"
        navigationPanel: root.navigationPanel
        navigationRowStart: showCourtesyClef.navigation.row + 1

        model: [
            { text: qsTrc("propertiespanel", "Auto"), value: 0, title: qsTrc("propertiespanel", "Auto") },
            { text: qsTrc("propertiespanel", "Before"), value: 1, title: qsTrc("propertiespanel", "Before") },
            { text: qsTrc("propertiespanel", "After"), value: 2, title: qsTrc("propertiespanel", "After") }
        ]
    }
}
