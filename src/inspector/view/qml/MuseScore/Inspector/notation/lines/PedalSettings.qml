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
import MuseScore.UiComponents 1.0
import MuseScore.Inspector 1.0

import "../../common"
import "internal"

Column {
    id: root

    property QtObject model: null

    objectName: "PedalSettings"

    spacing: 12

    CheckBox {
        isIndeterminate: root.model && root.model.showPedalSymbol.isUndefined
        checked: root.model && !isIndeterminate && root.model.showPedalSymbol.value

        text: qsTrc("inspector", "Show pedal symbol")

        onClicked: {
            root.model.showPedalSymbol.value = !checked
        }
    }

    LineTypeSection {
        endHookType: root.model ? root.model.endHookType : null
        thickness: root.model ? root.model.thickness : null
        hookHeight: root.model ? root.model.hookHeight : null
        possibleEndHookTypes: root.model ? root.model.possibleEndHookTypes() : null
    }

    CheckBox {
        isIndeterminate: root.model && root.model.showLineWithRosette.isUndefined
        checked: root.model && !isIndeterminate && root.model.showLineWithRosette.value
        visible: root.model & root.model.showLineWithRosetteVisible

        text: qsTrc("inspector", "Show line with rosette")

        onClicked: {
            root.model.showLineWithRosette.value = !checked
        }
    }

    SeparatorLine { anchors.margins: -10 }

    LineStyleSection {
        lineStyle: root.model ? root.model.lineStyle : null
        dashLineLength: root.model ? root.model.dashLineLength : null
        dashGapLength: root.model ? root.model.dashGapLength : null
    }
}
