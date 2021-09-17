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
import MuseScore.Ui 1.0
import "../../../common"

FocusableItem {
    id: root

    property QtObject model: null

    implicitHeight: contentColumn.height
    width: parent.width

    Column {
        id: contentColumn

        width: parent.width

        spacing: 12

        CheckBox {
            isIndeterminate: root.model ? root.model.isLineVisible.isUndefined : false
            checked: root.model && !isIndeterminate ? root.model.isLineVisible.value : false
            text: qsTrc("inspector", "Show line")

            onClicked: { root.model.isLineVisible.value = !checked }
        }

        LineTypeSection {
           endHookType: root.model ? root.model.endHookType : null
           thickness: root.model ? root.model.thickness : null
           hookHeight: root.model ? root.model.hookHeight : null

           possibleEndHookTypes: [
               { iconCode: IconCode.LINE_NORMAL, value: CrescendoTypes.HOOK_TYPE_NONE },
               { iconCode: IconCode.LINE_WITH_END_HOOK, value: CrescendoTypes.HOOK_TYPE_90 },
               { iconCode: IconCode.LINE_WITH_ANGLED_END_HOOK, value: CrescendoTypes.HOOK_TYPE_45 },
               { iconCode: IconCode.LINE_WITH_T_LIKE_END_HOOK, value: CrescendoTypes.HOOK_TYPE_T_LIKE }
           ]
        }

        SeparatorLine { anchors.margins: -10 }

        LineStyleSection {
            lineStyle: root.model ? root.model.lineStyle : null
            dashLineLength: root.model ? root.model.dashLineLength : null
            dashGapLength: root.model ? root.model.dashGapLength : null
        }

        SeparatorLine { anchors.margins: -10 }

        PlacementSection {
            propertyItem: root.model ? root.model.placement : null
        }
    }
}

