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
import QtQuick 2.15

import Muse.UiComponents 1.0
import MuseScore.Inspector 1.0

import "../../../common"

ExpandableBlank {
    id: root

    property QtObject model: null

    property int navigationRowEnd: contentItem.navigationRowEnd

    enabled: model ? !model.isEmpty : false

    title: model ? model.title : ""

    width: parent.width

    contentItemComponent: Column {
        property int navigationRowEnd: velocityChangeTypeSection.navigationRowEnd

        height: implicitHeight
        width: parent.width

        spacing: 12

        SpinBoxPropertyView {
            id: velocityChangeSection

            navigationName: "Velocity change"
            navigationPanel: root.navigation.panel
            navigationRowStart: root.navigation.row + 1

            titleText: qsTrc("inspector", "Velocity change")
            propertyItem: root.model ? root.model.velocityChange : null

            step: 1
            decimals: 0
            maxValue: 127
            minValue: 0
        }

        DropdownPropertyView {
            id: velocityChangeTypeSection

            navigationName: "Changes in dynamics range"
            navigationPanel: root.navigation.panel
            navigationRowStart: velocityChangeSection.navigationRowEnd + 1

            titleText: qsTrc("inspector", "Changes in dynamics range")
            propertyItem: root.model ? root.model.velocityChangeType : null

            model: [
                { text: qsTrc("inspector", "Linear (default)"), value: Hairpin.VELOCITY_EASING_LINEAR },
                { text: qsTrc("inspector", "Exponential"), value: Hairpin.VELOCITY_EASING_EXPONENTIAL },
                { text: qsTrc("inspector", "Ease in"), value: Hairpin.VELOCITY_EASING_IN },
                { text: qsTrc("inspector", "Ease out"), value: Hairpin.VELOCITY_EASING_OUT },
                { text: qsTrc("inspector", "Ease in and out"), value: Hairpin.VELOCITY_EASING_IN_OUT }
            ]
        }
    }
}

