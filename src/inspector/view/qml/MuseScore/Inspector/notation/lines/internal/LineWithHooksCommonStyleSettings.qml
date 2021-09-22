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
import MuseScore.Ui 1.0
import MuseScore.Inspector 1.0

import "../../../common"

Column {
    id: root

    property QtObject model: null

    width: parent.width

    spacing: 12

    Column {
        width: parent.width

        spacing: 6

        CheckBox {
            id: showLineCheckBox

            isIndeterminate: root.model && root.model.isLineVisible.isUndefined
            checked: root.model && !isIndeterminate && root.model.isLineVisible.value
            visible: root.model && root.model.isLineVisible.isVisible

            text: qsTrc("inspector", "Show line")

            onClicked: {
                root.model.isLineVisible.value = !checked
            }
        }

        CheckBox {
            isIndeterminate: root.model && root.model.allowDiagonal.isUndefined
            checked: root.model && !isIndeterminate && root.model.allowDiagonal.value
            visible: root.model && root.model.allowDiagonal.isVisible

            text: qsTrc("inspector", "Allow diagonal")

            onClicked: {
                root.model.allowDiagonal.value = !checked
            }
        }
    }

    LineTypeSection {
       endHookType: root.model ? root.model.endHookType : null
       thickness: root.model ? root.model.thickness : null
       hookHeight: root.model ? root.model.hookHeight : null
       possibleEndHookTypes: root.model ? root.model.possibleEndHookTypes() : null
    }

    SeparatorLine { anchors.margins: -10 }

    LineStyleSection {
        lineStyle: root.model ? root.model.lineStyle : null
        dashLineLength: root.model ? root.model.dashLineLength : null
        dashGapLength: root.model ? root.model.dashGapLength : null
    }

    SeparatorLine { anchors.margins: -10; visible: placementSection.visible }

    PlacementSection {
        id: placementSection

        propertyItem: root.model ? root.model.placement : null
    }
}
