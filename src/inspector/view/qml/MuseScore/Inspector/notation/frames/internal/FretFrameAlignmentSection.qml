/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import MuseScore.Inspector 1.0

import "../../../common"

InspectorPropertyView {
    id: root

    titleText: qsTrc("inspector", "Alignment")

    height: childrenRect.height
    width: parent.width

    navigationName: "AlignmentMenu"
    navigationRowEnd: horizontalAlignmentButtonList.navigationRowEnd

    function focusOnFirst() {
        horizontalAlignmentButtonList.itemAtIndex(0).navigation.requestActive()
    }

    RadioButtonGroup {
        id: horizontalAlignmentButtonList
        enabled: Boolean(root.propertyItem)

        property int navigationRowStart: root.navigationRowStart + 1
        property int navigationRowEnd: navigationRowStart + count

        height: 30

        model: [
            {
                iconRole: IconCode.ALIGN_LEFT,
                typeRole: CommonTypes.LEFT,
                title: qsTrc("inspector", "Align left"),
                description: qsTrc("inspector", "Align left edge of legend to reference point")
            },
            {
                iconRole: IconCode.ALIGN_HORIZONTAL_CENTER,
                typeRole: CommonTypes.HCENTER,
                title: qsTrc("inspector", "Align center"),
                description: qsTrc("inspector", "Align horizontal center of legend to reference point")
            },
            {
                iconRole: IconCode.ALIGN_RIGHT,
                typeRole: CommonTypes.RIGHT,
                title: qsTrc("inspector", "Align right"),
                description: qsTrc("inspector", "Align right edge of legend to reference point")
            }
        ]

        delegate: FlatRadioButton {
            navigation.panel: root.navigationPanel
            navigation.name: "HAlign" + model.index
            navigation.row: horizontalAlignmentButtonList.navigationRowStart + model.index
            navigation.accessible.name: modelData.title
            navigation.accessible.description: modelData.description

            toolTipTitle: modelData.title
            toolTipDescription: modelData.description

            width: 30
            transparent: true

            iconCode: modelData.iconRole
            checked: root.propertyItem && !root.propertyItem.isUndefined && (root.propertyItem.value === modelData.typeRole)
            onToggled: {
                root.propertyItem.value = modelData.typeRole
            }
        }
    }
}
