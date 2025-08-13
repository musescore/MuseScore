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
import Muse.Ui 1.0
import MuseScore.Inspector 1.0

import "../../../common"

Item {
    id: root

    property PropertyItem gapAbove: null
    property PropertyItem gapBelow: null
    property PropertyItem notationGapAbove: null
    property PropertyItem notationGapBelow: null

    property NavigationPanel navigationPanel: null
    property int navigationRowStart: 1
    property int navigationRowEnd: gapBelow.navigationRowEnd

    height: childrenRect.height
    width: parent.width

    function focusOnFirst() {
        gapAbove.focusOnFirst()
    }

    StyledTextLabel {
        id: gapToStaff
        horizontalAlignment: Qt.AlignLeft
        text: qsTrc("inspector", "Gap to staff/frames")
        font: ui.theme.bodyBoldFont
    }

    SpinBoxPropertyView {
        id: gapAbove
        anchors.top: gapToStaff.bottom
        anchors.topMargin: 4
        anchors.left: parent.left
        anchors.right: parent.horizontalCenter
        anchors.rightMargin: 4

        titleText: qsTrc("inspector", "Above")
        propertyItem: root.gapAbove

        icon: IconCode.GAP_ABOVE
        measureUnitsSymbol: qsTrc("global", "sp")

        navigationPanel: root.navigationPanel
        navigationRowStart: root.navigationRowStart + 1
    }

    SpinBoxPropertyView {
        id: gapBelow
        anchors.top: gapAbove.top
        anchors.left: parent.horizontalCenter
        anchors.leftMargin: 4
        anchors.right: parent.right

        titleText: qsTrc("inspector", "Below")
        propertyItem: root.gapBelow

        icon: IconCode.GAP_BELOW
        measureUnitsSymbol: qsTrc("global", "sp")

        navigationPanel: root.navigationPanel
        navigationRowStart: gapAbove.navigationRowEnd + 1
    }

    StyledTextLabel {
        id: notationPadding
        anchors.top: gapAbove.bottom
        anchors.topMargin: 12
        horizontalAlignment: Qt.AlignLeft
        text: qsTrc("inspector", "Clearance for notation")
        font: ui.theme.bodyBoldFont
    }

    SpinBoxPropertyView {
        id: gapNotationAbove
        anchors.top: notationPadding.bottom
        anchors.topMargin: 4
        anchors.left: parent.left
        anchors.right: parent.horizontalCenter
        anchors.rightMargin: 4

        titleText: qsTrc("inspector", "Above")
        propertyItem: root.notationGapAbove

        icon: IconCode.GAP_ABOVE
        measureUnitsSymbol: qsTrc("global", "sp")

        navigationPanel: root.navigationPanel
        navigationRowStart: root.navigationRowStart + 1
    }

    SpinBoxPropertyView {
        id: gapNotationBelow
        anchors.top: gapNotationAbove.top
        anchors.left: parent.horizontalCenter
        anchors.leftMargin: 4
        anchors.right: parent.right

        titleText: qsTrc("inspector", "Below")
        propertyItem: root.notationGapBelow

        icon: IconCode.GAP_BELOW
        measureUnitsSymbol: qsTrc("global", "sp")

        navigationPanel: root.navigationPanel
        navigationRowStart: gapAbove.navigationRowEnd + 1
    }
}
