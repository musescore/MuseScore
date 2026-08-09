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

    required property OrganPedalMarkSettingsModel model

    property NavigationPanel navigationPanel: null
    property int navigationRowStart: 1

    objectName: "OrganPedalMarkSettings"

    spacing: 12

    PlacementSection {
        id: pedalMarkPlacement
        titleText: qsTrc("propertiespanel", "Placement")
        propertyItem: root.model.placement

        height: implicitHeight

        navigationPanel: root.navigationPanel
        navigationRowStart: root.navigationRowStart

        isModified: false
    }

    SpinBoxPropertyView {
        id: pedalMarkSize

        anchors.left: parent.left

        navigationName: "Pedal mark size"
        navigationPanel: root.navigationPanel
        navigationRowStart: pedalMarkPlacement.navigationRowEnd + 1

        titleText: qsTrc("propertiespanel", "Size")
        measureUnitsSymbol: "pt"
        propertyItem: root.model.size

        decimals: 0
        step: 1
        minValue: 0
        maxValue: 1000
    }

    SeparatorLine { }

    DropdownPropertyView {
        id: pedalMarkStyle
        titleText: qsTrc("propertiespanel", "Pedal mark style")
        propertyItem: root.model.style

        navigationName: "Text style"
        navigationPanel: root.navigationPanel
        navigationRowStart: pedalMarkSize.navigationRowEnd + 1

        height: implicitHeight

        model: root.model.textStyles
    }
}
