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

import "../../../common"

Item {
    id: root

    property PropertyItem frameLeftMargin: null
    property PropertyItem frameRightMargin: null

    property NavigationPanel navigationPanel: null
    property int navigationRowStart: 1
    property int navigationRowEnd: rightMarginsSection.navigationRowEnd

    height: childrenRect.height
    width: parent.width

    SpinBoxPropertyView {
        id: leftMarginsSection
        anchors.left: parent.left
        anchors.right: parent.horizontalCenter
        anchors.rightMargin: 4

        titleText: qsTrc("inspector", "Left padding")
        propertyItem: root.frameLeftMargin

        icon: IconCode.LEFT_MARGIN
        measureUnitsSymbol: qsTrc("global", "mm")

        navigationPanel: root.navigationPanel
        navigationRowStart: root.navigationRowStart + 1
    }

    SpinBoxPropertyView {
        id: rightMarginsSection
        anchors.left: parent.horizontalCenter
        anchors.leftMargin: 4
        anchors.right: parent.right

        titleText: qsTrc("inspector", "Right padding")
        propertyItem: root.frameRightMargin

        icon: IconCode.RIGHT_MARGIN
        measureUnitsSymbol: qsTrc("global", "mm")

        navigationPanel: root.navigationPanel
        navigationRowStart: leftMarginsSection.navigationRowEnd + 1
    }
}
