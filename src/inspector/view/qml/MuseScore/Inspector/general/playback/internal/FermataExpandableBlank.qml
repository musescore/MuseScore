/**
 * // SPDX-License-Identifier: GPL-3.0-only
 * // MuseScore-CLA-applies
 * //=============================================================================
 * //  MuseScore
 * //  Music Composition & Notation
 * //
 * //  Copyright (C) 2021 MuseScore BVBA and others
 * //
 * //  This program is free software: you can redistribute it and/or modify
 * //  it under the terms of the GNU General Public License version 3 as
 * //  published by the Free Software Foundation.
 * //
 * //  This program is distributed in the hope that it will be useful,
 * //  but WITHOUT ANY WARRANTY; without even the implied warranty of
 * //  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * //  GNU General Public License for more details.
 * //
 * //  You should have received a copy of the GNU General Public License
 * //  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * //============================================================================= **/
import QtQuick 2.9
import MuseScore.Inspector 1.0
import MuseScore.UiComponents 1.0
import "../../../common"

ExpandableBlank {
    id: root

    property QtObject model: null

    enabled: model ? !model.isEmpty : false

    title: model ? model.title : ""

    width: parent.width

    contentItemComponent: InspectorPropertyView {
        anchors.left: parent.left
        anchors.right: parent.horizontalCenter
        anchors.rightMargin: 2

        titleText: qsTrc("inspector", "Time stretch")
        propertyItem: model ? model.timeStretch : null

        IncrementalPropertyControl {
            id: timeStretchControl

            iconMode: iconModeEnum.hidden

            measureUnitsSymbol: "%"
            isIndeterminate: model ? model.timeStretch.isUndefined : false
            currentValue: model ? model.timeStretch.value : 0

            step: 1
            decimals: 0
            maxValue: 400
            minValue: 0
            validator: IntInputValidator {
                top: timeStretchControl.maxValue
                bottom: timeStretchControl.minValue
            }

            onValueEdited: { model.timeStretch.value = newValue }
        }
    }
}
