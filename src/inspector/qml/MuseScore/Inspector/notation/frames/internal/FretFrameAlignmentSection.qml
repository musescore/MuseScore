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
pragma ComponentBehavior: Bound

import QtQuick

import Muse.Ui
import Muse.UiComponents
import MuseScore.Inspector

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
                iconCode: IconCode.ALIGN_LEFT,
                value: CommonTypes.LEFT,
                title: qsTrc("inspector", "Align left"),
                description: qsTrc("inspector", "Align left edge of legend to reference point")
            },
            {
                iconCode: IconCode.ALIGN_HORIZONTAL_CENTER,
                value: CommonTypes.HCENTER,
                title: qsTrc("inspector", "Align center"),
                description: qsTrc("inspector", "Align horizontal center of legend to reference point")
            },
            {
                iconCode: IconCode.ALIGN_RIGHT,
                value: CommonTypes.RIGHT,
                title: qsTrc("inspector", "Align right"),
                description: qsTrc("inspector", "Align right edge of legend to reference point")
            }
        ]

        delegate: FlatRadioButton {
            required iconCode
            required property int value
            required property string title
            required property string description
            required property int index

            navigation.panel: root.navigationPanel
            navigation.name: "HAlign" + index
            navigation.row: horizontalAlignmentButtonList.navigationRowStart + index
            navigation.accessible.name: title
            navigation.accessible.description: description

            toolTipTitle: title
            toolTipDescription: description

            width: 30
            transparent: true

            checked: root.propertyItem && !root.propertyItem.isUndefined && (root.propertyItem.value === value)
            onToggled: {
                root.propertyItem.value = value
            }
        }
    }
}
