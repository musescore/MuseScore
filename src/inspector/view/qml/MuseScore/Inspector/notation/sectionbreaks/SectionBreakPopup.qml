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
import QtQuick 2.9
import QtQuick.Controls 2.2
import MuseScore.Inspector 1.0
import MuseScore.UiComponents 1.0
import "../../common"

StyledPopupView {
    id: root

    property QtObject model: null

    contentHeight: contentColumn.implicitHeight

    Column {
        id: contentColumn

        width: parent.width

        spacing: 12

        InspectorPropertyView {
            titleText: qsTrc("inspector", "Pause before new section starts")
            propertyItem: model ? model.pauseDuration : null

            IncrementalPropertyControl {
                isIndeterminate: model ? model.pauseDuration.isUndefined : false
                currentValue: model ? model.pauseDuration.value : 0
                iconMode: iconModeEnum.hidden
                maxValue: 999
                minValue: 0
                step: 0.5
                measureUnitsSymbol: qsTrc("inspector", "s")

                onValueEdited: { model.pauseDuration.value = newValue }
            }
        }

        CheckBox {
            isIndeterminate: model ? model.shouldStartWithLongInstrNames.isUndefined : false
            checked: model && !isIndeterminate ? model.shouldStartWithLongInstrNames.value : false
            text: qsTrc("inspector", "Start new section with long instrument names")

            onClicked: { model.shouldStartWithLongInstrNames.value = !checked }
        }

        CheckBox {
            isIndeterminate: model ? model.shouldResetBarNums.isUndefined : false
            checked: model && !isIndeterminate ? model.shouldResetBarNums.value : false
            text: qsTrc("inspector", "Reset bar numbers for new section")

            onClicked: { model.shouldResetBarNums.value = !checked }
        }
    }
}
