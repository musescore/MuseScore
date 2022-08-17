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

import MuseScore.Inspector 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0

import "../../../common"

Column {
    id: root

    property PropertyItem lineStyle: null
    property alias possibleLineStyles: styleSection.model

    property PropertyItem dashLineLength: null
    property PropertyItem dashGapLength: null

    property NavigationPanel navigationPanel: null
    property int navigationRowStart: 1
    property int navigationRowEnd: gapSection.navigationRowEnd

    width: parent.width

    spacing: 12

    function focusOnFirst() {
        styleSection.focusOnFirst()
    }

    FlatRadioButtonGroupPropertyView {
        id: styleSection
        titleText: qsTrc("inspector", "Style")
        propertyItem: root.lineStyle

        navigationPanel: root.navigationPanel
        navigationRowStart: root.navigationRowStart

        model: [
            { iconCode: IconCode.LINE_NORMAL, value: LineTypes.LINE_STYLE_SOLID, title: qsTrc("inspector", "Normal") },
            { iconCode: IconCode.LINE_DASHED, value: LineTypes.LINE_STYLE_DASHED, title: qsTrc("inspector", "Dashed") },
            { iconCode: IconCode.LINE_DOTTED, value: LineTypes.LINE_STYLE_DOTTED, title: qsTrc("inspector", "Dotted") },
        ]
    }

    Item {
        height: childrenRect.height
        width: parent.width

        SpinBoxPropertyView {
            id: dashSection
            anchors.left: parent.left
            anchors.right: parent.horizontalCenter
            anchors.rightMargin: 2

            visible: root.dashLineLength && root.dashLineLength.isEnabled
            height: visible ? implicitHeight : 0

            titleText: qsTrc("inspector", "Dash")
            propertyItem: root.dashLineLength

            step: 0.1
            maxValue: 100
            minValue: 0.1
            decimals: 2

            navigationName: "Dash"
            navigationPanel: root.navigationPanel
            navigationRowStart: styleSection.navigationRowEnd + 1
        }

        SpinBoxPropertyView {
            id: gapSection
            anchors.left: parent.horizontalCenter
            anchors.leftMargin: 2
            anchors.right: parent.right

            visible: root.dashGapLength && root.dashGapLength.isEnabled
            height: visible ? implicitHeight : 0

            titleText: qsTrc("inspector", "Gap")
            propertyItem: root.dashGapLength

            step: 0.1
            maxValue: 100
            minValue: 0.1
            decimals: 2

            navigationName: "Gap"
            navigationPanel: root.navigationPanel
            navigationRowStart: dashSection.navigationRowEnd + 1
        }
    }
}

