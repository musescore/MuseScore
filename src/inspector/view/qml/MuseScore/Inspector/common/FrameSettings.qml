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

    property string navigationName: "FrameSettings"
    property NavigationPanel navigationPanel: null
    property int navigationRowStart: 0
    readonly property int navigationRowEnd: cornerRadiusSection.navigationRowEnd

    required property PropertyItem frameType
    required property PropertyItem frameBorderColor
    required property PropertyItem frameFillColor
    required property PropertyItem frameThickness
    required property PropertyItem frameMargin
    required property PropertyItem frameCornerRadius

    FlatRadioButtonGroupPropertyView {
        id: frameSection
        titleText: qsTrc("inspector", "Frame")
        propertyItem: root.frameType ? root.frameType : null

        navigationName: "FrameMenu"
        navigationPanel: root.navigationPanel
        navigationRowStart: root.navigationRowStart

        model: [
            { text: qsTrc("inspector", "None"), value: TextTypes.FRAME_TYPE_NONE, titleRole: qsTrc("inspector", "None") },
            { iconCode: IconCode.FRAME_SQUARE, value: TextTypes.FRAME_TYPE_SQUARE, titleRole: qsTrc("inspector", "Rectangle") },
            { iconCode: IconCode.FRAME_CIRCLE, value: TextTypes.FRAME_TYPE_CIRCLE, titleRole: qsTrc("inspector", "Circle") }
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
            navigationRowStart: frameSection.navigationRowEnd + 1

            visible: root.frameBorderColor ? root.frameBorderColor.isEnabled : false
            height: visible ? implicitHeight : 0

            titleText: qsTrc("inspector", "Border")
            propertyItem: root.frameBorderColor
        }

        ColorSection {
            id: highlightColorSection
            anchors.left: parent.horizontalCenter
            anchors.leftMargin: 2
            anchors.right: parent.right

            navigationName: "HighlightColorMenu"
            navigationPanel: root.navigationPanel
            navigationRowStart: borderColorSection.navigationRowEnd + 1

            visible: root.frameFillColor ? root.frameFillColor.isEnabled : false
            height: visible ? implicitHeight : 0

            titleText: qsTrc("inspector", "Fill color")
            propertyItem: root.frameFillColor
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

            visible: root.frameThickness ? root.frameThickness.isEnabled : false
            height: visible ? implicitHeight : 0

            titleText: qsTrc("inspector", "Thickness")
            propertyItem: root.frameThickness

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

            visible: root.frameMargin ? root.frameMargin.isEnabled : false
            height: visible ? implicitHeight : 0

            titleText: qsTrc("inspector", "Padding")
            propertyItem: root.frameMargin

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

        visible: root.frameCornerRadius ? root.frameCornerRadius.isEnabled : false
        height: visible ? implicitHeight : 0

        titleText: qsTrc("inspector", "Corner radius")
        propertyItem: root.frameCornerRadius

        step: 1
        decimals: 2
        minValue: 0
        maxValue: 100
    }
}
