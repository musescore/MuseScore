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

import MuseScore.UiComponents 1.0
import MuseScore.Inspector 1.0
import "../../common"

Column {
    id: root

    property QtObject model: null

    objectName: "SectionBreakSettings"

    spacing: 12

    InspectorPropertyView {
        titleText: qsTrc("inspector", "Pause before new section starts")
        propertyItem: root.model ? root.model.pauseDuration : null

        IncrementalPropertyControl {
            isIndeterminate: root.model ? root.model.pauseDuration.isUndefined : false
            currentValue: root.model ? root.model.pauseDuration.value : 0
            iconMode: iconModeEnum.hidden
            maxValue: 999
            minValue: 0
            step: 0.5
            measureUnitsSymbol: qsTrc("inspector", "s")

            onValueEdited: { root.model.pauseDuration.value = newValue }
        }
    }

    CheckBox {
        isIndeterminate: root.model ? root.model.shouldStartWithLongInstrNames.isUndefined : false
        checked: root.model && !isIndeterminate ? root.model.shouldStartWithLongInstrNames.value : false
        text: qsTrc("inspector", "Start new section with long instrument names")

        onClicked: { root.model.shouldStartWithLongInstrNames.value = !checked }
    }

    CheckBox {
        isIndeterminate: root.model ? root.model.shouldResetBarNums.isUndefined : false
        checked: root.model && !isIndeterminate ? root.model.shouldResetBarNums.value : false
        text: qsTrc("inspector", "Reset bar numbers for new section")

        onClicked: { root.model.shouldResetBarNums.value = !checked }
    }
}
