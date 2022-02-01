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
import QtQuick.Layouts 1.15

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Inspector 1.0

import "../../../common"

Column {
    id: root

    property PropertyItem startHookType: null
    property PropertyItem endHookType: null
    property PropertyItem thickness: null
    property PropertyItem hookHeight: null

    property alias possibleStartHookTypes: startHookButtonGroup.model
    property alias possibleEndHookTypes: lineTypeButtonGroup.model

    property NavigationPanel navigationPanel: null
    property int navigationRowStart: 1
    property int navigationRowEnd: hookHeightSection.navigationRowEnd

    width: parent.width

    spacing: 12

    FlatRadioButtonGroupPropertyView {
        id: lineTypeButtonGroup

        titleText: qsTrc("inspector", "Line type")
        propertyItem: root.endHookType

        navigationPanel: root.navigationPanel
        navigationRowStart: root.navigationRowStart
    }

    FlatRadioButtonGroupPropertyView {
        id: startHookButtonGroup

        titleText: qsTrc("inspector", "Start hook")
        propertyItem: root.startHookType

        navigationPanel: root.navigationPanel
        navigationRowStart: lineTypeButtonGroup.navigationRowEnd + 1
    }

    Item {
        height: childrenRect.height
        width: parent.width

        SpinBoxPropertyView {
            id: thicknessSection
            anchors.left: parent.left
            anchors.right: parent.horizontalCenter
            anchors.rightMargin: 2

            titleText: qsTrc("inspector", "Thickness")
            propertyItem: root.thickness

            step: 0.01
            maxValue: 10.00
            minValue: 0.01
            decimals: 2

            navigationName: "Thickness"
            navigationPanel: root.navigationPanel
            navigationRowStart: startHookButtonGroup.navigationRowEnd + 1
        }

        SpinBoxPropertyView {
            id: hookHeightSection
            anchors.left: parent.horizontalCenter
            anchors.leftMargin: 2
            anchors.right: parent.right

            titleText: qsTrc("inspector", "Hook height")
            propertyItem: root.hookHeight

            step: 0.5
            maxValue: 10.0
            minValue: 0.1
            decimals: 2

            navigationName: "HookHeight"
            navigationPanel: root.navigationPanel
            navigationRowStart: thicknessSection.navigationRowEnd + 1
        }
    }
}
