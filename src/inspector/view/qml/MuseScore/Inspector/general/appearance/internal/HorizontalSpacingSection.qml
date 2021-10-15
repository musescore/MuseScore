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

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Inspector 1.0

import "../../../common"

Item {
    id: root

    property PropertyItem leadingSpace: null
    property PropertyItem barWidth: null

    property NavigationPanel navigationPanel: null
    property int navigationRowStart: 0
    property int navigationRowEnd: barWidth.navigationRowEnd

    function focusOnFirst() {
        leadingValue.focusOnFirst()
    }

    height: childrenRect.height
    width: parent.width

    SpinBoxPropertyView {
        id: leadingValue

        anchors.left: parent.left
        anchors.right: parent.horizontalCenter
        anchors.rightMargin: 4

        navigationName: "Leading"
        navigationPanel: root.navigationPanel
        navigationRowStart: root.navigationRowStart + 1

        titleText: qsTrc("inspector", "Leading")
        propertyItem: root.leadingSpace

        icon: IconCode.HORIZONTAL
    }

    SpinBoxPropertyView {
        id: barWidth
        anchors.left: parent.horizontalCenter
        anchors.leftMargin: 4
        anchors.right: parent.right

        navigationName: "Bar width"
        navigationPanel: root.navigationPanel
        navigationRowStart: leadingValue.navigationRowEnd + 1

        titleText: qsTrc("inspector", "Bar width")
        propertyItem: root.barWidth

        icon: IconCode.HORIZONTAL
    }
}
