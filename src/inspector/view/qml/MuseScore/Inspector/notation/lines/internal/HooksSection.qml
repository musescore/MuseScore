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
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import MuseScore.Inspector 1.0

import "../../../common"

Column {
    id: root

    property PropertyItem startHookType: null
    property PropertyItem endHookType: null
    property PropertyItem startHookHeight: null
    property PropertyItem endHookHeight: null

    property alias possibleStartHookTypes: startHookButtonGroup.model
    property alias possibleEndHookTypes: endHookButtonGroup.model

    property NavigationPanel navigationPanel: null
    property int navigationRowStart: 1
    property int navigationRowEnd: endHookHeightSection.navigationRowEnd

    // NOTE can't bind to `visible` property of children, because children are always invisible when parent is invisible
    visible: startHookButtonGroup.isUseful || endHookButtonGroup.isUseful || hookHeightSections.isUseful

    width: parent.width

    spacing: 12

    FlatRadioButtonGroupPropertyView {
        id: startHookButtonGroup

        readonly property bool isUseful: Boolean(root.possibleStartHookTypes) && root.possibleStartHookTypes.length > 1

        visible: isUseful

        titleText: qsTrc("inspector", "Start hook")
        propertyItem: root.startHookType

        navigationPanel: root.navigationPanel
        navigationRowStart: root.navigationRowStart
    }

    FlatRadioButtonGroupPropertyView {
        id: endHookButtonGroup

        readonly property bool isUseful: Boolean(root.possibleEndHookTypes) && root.possibleEndHookTypes.length > 1

        visible: isUseful

        titleText: qsTrc("inspector", "End hook")
        propertyItem: root.endHookType

        navigationPanel: root.navigationPanel
        navigationRowStart: startHookButtonGroup.navigationRowEnd + 1
    }

    Item {
        id: hookHeightSections

        readonly property bool isUseful: (root.startHookHeight && root.startHookHeight.isVisible)
                                         || (root.endHookHeight && root.endHookHeight.isVisible)

        visible: isUseful

        height: childrenRect.height
        width: parent.width

        SpinBoxPropertyView {
            id: startHookHeightSection
            anchors.left: parent.left
            anchors.right: parent.horizontalCenter
            anchors.rightMargin: 2

            titleText: qsTrc("inspector", "Start hook height")
            propertyItem: root.startHookHeight

            step: 0.5
            maxValue: 1000.0
            minValue: -1000.0
            decimals: 2

            navigationName: "StartHookHeight"
            navigationPanel: root.navigationPanel
            navigationRowStart: endHookButtonGroup.navigationRowEnd + 1
        }

        SpinBoxPropertyView {
            id: endHookHeightSection
            anchors.left: parent.horizontalCenter
            anchors.leftMargin: 2
            anchors.right: parent.right

            titleText: qsTrc("inspector", "End hook height")
            propertyItem: root.endHookHeight

            step: 0.5
            maxValue: 1000.0
            minValue: -1000.0
            decimals: 2

            navigationName: "EndHookHeight"
            navigationPanel: root.navigationPanel
            navigationRowStart: startHookHeightSection.navigationRowEnd + 1
        }
    }
}
