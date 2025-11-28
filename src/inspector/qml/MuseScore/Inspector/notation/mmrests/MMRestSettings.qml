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
import QtQuick

import Muse.Ui
import Muse.UiComponents
import MuseScore.Inspector

import "../../common"

Column {
    id: root

    required property MMRestSettingsModel model

    property NavigationPanel navigationPanel: null
    property int navigationRowStart: 1

    objectName: "MMRestSettings"

    spacing: 12

    function focusOnFirst() {
        numberVisibilityCheckBox.navigation.requestActive()
    }

    PropertyCheckBox {
        id: numberVisibilityCheckBox
        text: qsTrc("inspector", "Show number")
        propertyItem: root.model ? root.model.isNumberVisible : null
        enabled: root.model ? root.model.areNumberOptionsEnabled : true

        navigation.name: "NumberVisibilityCheckBox"
        navigation.panel: root.navigationPanel
        navigation.row: root.navigationRowStart + 1
    }

    SpinBoxPropertyView {
        enabled: (root.model ? root.model.areNumberOptionsEnabled : true) && numberVisibilityCheckBox.checked
        titleText: qsTrc("inspector", "Number offset")
        propertyItem: root.model ? root.model.numberPosition : null

        icon: IconCode.VERTICAL

        minValue: -99.0
        maxValue: 99.0
        step: 0.5
        decimals: 2
        measureUnitsSymbol: qsTrc("global", "sp")

        navigationPanel: root.navigationPanel
        navigationRowStart: root.navigationRowStart + 2
    }
}
