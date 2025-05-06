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

Column {
    id: root

    width: parent.width
    spacing: 12

    property string navigationName: "BorderSettings"
    property NavigationPanel navigationPanel: null
    property int navigationRowStart: 0
    readonly property int navigationRowEnd: cornerRadiusSection.navigationRowEnd

    required property PropertyItem borderType
    required property PropertyItem borderColor
    required property PropertyItem borderFillColor
    required property PropertyItem borderThickness
    required property PropertyItem borderMargin
    required property PropertyItem borderCornerRadius

    FlatRadioButtonGroupPropertyView {
        id: borderSection
        titleText: qsTrc("inspector", "Border")
        propertyItem: root.borderType ? root.borderType : null

        navigationName: "BorderMenu"
        navigationPanel: root.navigationPanel
        navigationRowStart: root.navigationRowStart

        model: [
            { text: qsTrc("inspector", "None"), value: TextTypes.BORDER_TYPE_NONE, titleRole: qsTrc("inspector", "None") },
            { iconCode: IconCode.BORDER_SQUARE, value: TextTypes.BORDER_TYPE_SQUARE, titleRole: qsTrc("inspector", "Rectangle") },
            { iconCode: IconCode.BORDER_CIRCLE, value: TextTypes.BORDER_TYPE_CIRCLE, titleRole: qsTrc("inspector", "Circle") }
        ]
    }

    Item {
        height: childrenRect.height
        width: parent.width

        ColorSection {
            id: borderColorSection
            anchors.left: parent.left
            anchors.right: parent.horizontalCenter
            anchors.rightMargin: 2

            navigationName: "BorderColorMenu"
            navigationPanel: root.navigationPanel
            navigationRowStart: borderSection.navigationRowEnd + 1

            visible: root.borderColor ? root.borderColor.isEnabled : false
            height: visible ? implicitHeight : 0

            titleText: qsTrc("inspector", "Border")
            propertyItem: root.borderColor
        }

        ColorSection {
            id: highlightColorSection
            anchors.left: parent.horizontalCenter
            anchors.leftMargin: 2
            anchors.right: parent.right

            navigationName: "HighlightColorMenu"
            navigationPanel: root.navigationPanel
            navigationRowStart: borderColorSection.navigationRowEnd + 1

            visible: root.borderFillColor ? root.borderFillColor.isEnabled : false
            height: visible ? implicitHeight : 0

            titleText: qsTrc("inspector", "Fill color")
            propertyItem: root.borderFillColor
        }
    }

    Item {
        height: childrenRect.height
        width: parent.width

        SpinBoxPropertyView {
            id: thicknessSection
            anchors.left: parent.left
            anchors.right: parent.horizontalCenter
            anchors.rightMargin: 2

            navigationName: "Thickness"
            navigationPanel: root.navigationPanel
            navigationRowStart: highlightColorSection.navigationRowEnd + 1

            visible: root.borderThickness ? root.borderThickness.isEnabled : false
            height: visible ? implicitHeight : 0

            titleText: qsTrc("inspector", "Thickness")
            propertyItem: root.borderThickness

            step: 0.1
            minValue: 0
            maxValue: 5
        }

        SpinBoxPropertyView {
            id: marginSection
            anchors.left: parent.horizontalCenter
            anchors.leftMargin: 2
            anchors.right: parent.right

            navigationName: "Padding"
            navigationPanel: root.navigationPanel
            navigationRowStart: thicknessSection.navigationRowEnd + 1

            visible: root.borderMargin ? root.borderMargin.isEnabled : false
            height: visible ? implicitHeight : 0

            titleText: qsTrc("inspector", "Padding")
            propertyItem: root.borderMargin

            step: 0.1
            minValue: 0
            maxValue: 5
        }
    }

    SpinBoxPropertyView {
        id: cornerRadiusSection
        anchors.left: parent.left
        anchors.right: parent.horizontalCenter
        anchors.rightMargin: 2

        navigationName: "Corner radius"
        navigationPanel: root.navigationPanel
        navigationRowStart: marginSection.navigationRowEnd + 1

        visible: root.borderCornerRadius ? root.borderCornerRadius.isEnabled : false
        height: visible ? implicitHeight : 0

        titleText: qsTrc("inspector", "Corner radius")
        propertyItem: root.borderCornerRadius

        step: 1
        decimals: 2
        minValue: 0
        maxValue: 100
    }
}
