/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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
import "internal"

Column {
    id: root

    required property BoxLineSettingsModel model

    property NavigationPanel navigationPanel: null
    property int navigationRowStart: 1

    objectName: "BoxLineSettings"

    spacing: 12

    function focusOnFirst() {
        heightSection.focusOnFirst()
    }

    SpinBoxPropertyView {
        id: heightSection

        titleText: qsTrc("propertiespanel", "Height")
        propertyItem: root.model ? root.model.height : null

        step: 0.1
        minValue: root.model ? root.model.minHeight : 0
        maxValue: root.model ? root.model.maxHeight : 0
        decimals: 2
        measureUnitsSymbol: qsTrc("global", "sp")

        navigationName: "Height"
        navigationPanel: root.navigationPanel
        navigationRowStart: root.navigationRowStart
    }

    LineStyleSection {
        id: lineStyleSection

        thickness: root.model ? root.model.thickness : null
        lineStyle: root.model ? root.model.lineStyle : null
        dashLineLength: root.model ? root.model.dashLineLength : null
        dashGapLength: root.model ? root.model.dashGapLength : null

        navigationPanel: root.navigationPanel
        navigationRowStart: heightSection.navigationRowEnd + 1
    }

    SeparatorLine { anchors.margins: -12 }

    PlacementSection {
        propertyItem: root.model ? root.model.placement : null

        navigationPanel: root.navigationPanel
        navigationRowStart: lineStyleSection.navigationRowEnd + 1
    }
}
