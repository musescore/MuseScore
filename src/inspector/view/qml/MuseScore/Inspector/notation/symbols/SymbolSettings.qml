/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited
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

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import MuseScore.Inspector 1.0

import "../../common"

Column {
    id: root

    property QtObject model: null

    property NavigationPanel navigationPanel: null
    property int navigationRowStart: 1

    objectName: "SymbolSettings"

    spacing: 12

    function focusOnFirst() {
        scoreFontSection.focusOnFirst()
    }

    Item {
        height: childrenRect.height
        width: parent.width
        
        SpinBoxPropertyView {
            id: symbolSize

            anchors.left: parent.left
            anchors.right: parent.horizontalCenter
            anchors.rightMargin: 8

            navigationName: "Scale"
            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigationRowStart

            titleText: qsTrc("inspector", "Scale")
            measureUnitsSymbol: "%"
            propertyItem: root.model ? root.model.symbolSize : null

            decimals: 0
            step: 1
            minValue: 0
            maxValue: 1000
        }

        SpinBoxPropertyView {
            id: symAngle

            anchors.left: parent.horizontalCenter
            anchors.right: parent.right

            navigationName: "Rotation"
            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigationRowStart + 1

            titleText: qsTrc("inspector", "Rotation")
            measureUnitsSymbol: "°"
            propertyItem: root.model ? root.model.symAngle : null

            decimals: 0
            step: 1
            minValue: 0
            maxValue: 360
            wrap: true
        }
    }

    DropdownPropertyView {
        id: scoreFontSection
        titleText: qsTrc("inspector", "Font")
        propertyItem: root.model ? root.model.scoreFont : null

        navigationPanel: root.navigationPanel
        navigationRowStart: root.navigationRowStart + 2
        
        model: root.model ? root.model.symFonts : []
    }
}
