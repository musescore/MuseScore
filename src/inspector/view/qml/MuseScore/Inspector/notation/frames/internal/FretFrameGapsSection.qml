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

Item {
    id: root

    property PropertyItem columnGap: null
    property PropertyItem rowGap: null

    property NavigationPanel navigationPanel: null
    property int navigationRowStart: 1
    property int navigationRowEnd: rowGapView.navigationRowEnd

    height: childrenRect.height
    width: parent.width

    function focusOnFirst() {
        columnGapView.focusOnFirst()
    }

    SpinBoxPropertyView {
        id: columnGapView
        anchors.left: parent.left
        anchors.right: parent.horizontalCenter
        anchors.rightMargin: 4

        titleText: qsTrc("inspector", "Column gap")
        propertyItem: root.columnGap

        icon: IconCode.HORIZONTAL
        measureUnitsSymbol: qsTrc("global", "sp")

        navigationPanel: root.navigationPanel
        navigationRowStart: root.navigationRowStart + 1
    }

    SpinBoxPropertyView {
        id: rowGapView
        anchors.left: parent.horizontalCenter
        anchors.leftMargin: 4
        anchors.right: parent.right

        titleText: qsTrc("inspector", "Row gap")
        propertyItem: root.rowGap

        icon: IconCode.VERTICAL
        measureUnitsSymbol: qsTrc("global", "sp")

        navigationPanel: root.navigationPanel
        navigationRowStart: columnGapView.navigationRowEnd + 1
    }
}
