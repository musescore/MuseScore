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

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import MuseScore.Inspector 1.0

import "../../common"

Column {
    id: root
    width: parent.width
    height: childrenRect.height

    property QtObject model: null

    property NavigationPanel navigationPanel: null
    property int navigationRowStart: 1

    objectName: "DynamicsSettings"

    spacing: 12

    function focusOnFirst() {
        avoidBarLines.navigation.requestActive()
    }

    SpinBoxPropertyView {
        id: dynamicSize

        anchors.left: parent.left

        navigationName: "Scale"
        navigationPanel: root.navigationPanel
        navigationRowStart: root.navigationRowStart + 1

        titleText: qsTrc("inspector", "Scale")
        measureUnitsSymbol: qsTrc("global", "%")
        propertyItem: root.model ? root.model.dynamicSize : null

        decimals: 0
        step: 1
        minValue: 0
        maxValue: 1000
    }

    CheckBoxPropertyView {
        id: avoidBarLines

        navigationName: "Avoid barlines"
        navigationPanel: root.navigationPanel
        navigationRowStart: dynamicSize.navigationRowEnd + 1

        titleText: qsTrc("inspector", "Avoid barlines")
        propertyItem: root.model ? root.model.avoidBarLines : null
    }

    VoicesAndPositionSection {
        id: voicesAndPositionSection

        navigationPanel: root.navigationPanel
        navigationRowStart: avoidBarLines.navigationRowEnd + 1

        model: root.model
    }

    ExpandableBlank {
        id: showItem
        isExpanded: false
        title: isExpanded ? qsTrc("inspector", "Show less") : qsTrc("inspector", "Show more")

        width: parent.width

        navigation.panel: root.navigationPanel
        navigation.row: voicesAndPositionSection.navigationRowEnd + 1

        contentItemComponent: Column {
            width: parent.width
            spacing: root.spacing

            FlatRadioButtonGroupPropertyView {
                id: dynamicAlignmentGroup

                requestHeight: 55
                requestIconFontSize: 32

                titleText: qsTrc("inspector", "Alignment with notehead")
                propertyItem: root.model ? root.model.centerOnNotehead : null

                navigationPanel: root.navigationPanel
                navigationRowStart: showItem.navigation.row + 1

                model: [
                    { iconCode: IconCode.DYNAMIC_CENTER_1, value: true, title: qsTrc("inspector", "Center on notehead") },
                    { iconCode: IconCode.DYNAMIC_CENTER_2, value: false, title: qsTrc("inspector", "Use text centering") }
                ]
            }

            FrameSettings {
                id: frameSettings

                navigationPanel: root.navigationPanel
                navigationRowStart: dynamicAlignmentGroup.navigationRowEnd + 1

                frameType: root.model ? root.model.frameType : null
                frameBorderColor: root.model ? root.model.frameBorderColor : null
                frameFillColor: root.model ? root.model.frameFillColor : null
                frameThickness: root.model ? root.model.frameThickness : null
                frameMargin: root.model ? root.model.frameMargin : null
                frameCornerRadius: root.model ? root.model.frameCornerRadius : null
            }
        }
    }
}



