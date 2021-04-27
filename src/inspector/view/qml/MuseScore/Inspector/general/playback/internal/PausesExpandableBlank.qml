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

        navigation.name: "Pause time Menu"
        navigation.panel: root.navigation.panel
        navigation.column: root.navigation.column
        navigation.row: root.navigation.row + 1

        titleText: qsTrc("inspector", "Pause time")
        propertyItem: model ? model.pauseTime : null

        IncrementalPropertyControl {
            iconMode: iconModeEnum.hidden

            navigation.name: "Pause time Value"
            navigation.panel: root.navigation.panel
            navigation.column: root.navigation.column
            navigation.row: root.navigation.row + 2

            measureUnitsSymbol: "s"
            isIndeterminate: model ? model.pauseTime.isUndefined : false
            currentValue: model ? model.pauseTime.value : 0

            minValue: 0.0
            maxValue: 60.0

            onValueEdited: { model.pauseTime.value = newValue }
        }
    }
}
