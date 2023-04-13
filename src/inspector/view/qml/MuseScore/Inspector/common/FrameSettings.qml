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

Column {
    id: frameSettings

    width: parent.width
    spacing: 12

    property string navigationName: "FrameSettings"
    property NavigationPanel navigationPanel: null
    property int navigationRowStart: 0

    required property QtObject frameTypePropertyItem
    required property QtObject frameBorderColorPropertyItem
    required property QtObject frameFillColorPropertyItem
    required property QtObject frameThicknessPropertyItem
    required property QtObject frameMarginPropertyItem
    required property QtObject frameCornerRadiusPropertyItem

    FlatRadioButtonGroupPropertyView {
        id: frameSection
        titleText: qsTrc("inspector", "Frame")
        propertyItem: frameSettings.frameTypePropertyItem ? frameSettings.frameTypePropertyItem : null

        navigationName: "FrameMenu"
        navigationPanel: frameSettings.navigationPanel
        navigationRowStart: frameSettings.navigationRowStart

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
            navigationPanel: frameSettings.navigationPanel
            navigationRowStart: frameSection.navigationRowEnd + 1

            visible: frameSettings.frameBorderColorPropertyItem.isEnabled
            height: visible ? implicitHeight : 0

            titleText: qsTrc("inspector", "Border")
            propertyItem: frameSettings.frameBorderColorPropertyItem
        }

        ColorSection {
            id: highlightColorSection
            anchors.left: parent.horizontalCenter
            anchors.leftMargin: 2
            anchors.right: parent.right

            navigationName: "HighlightColorMenu"
            navigationPanel: frameSettings.navigationPanel
            navigationRowStart: borderColorSection.navigationRowEnd + 1

            visible: frameSettings.frameFillColorPropertyItem.isEnabled
            height: visible ? implicitHeight : 0

            titleText: qsTrc("inspector", "Fill color")
            propertyItem: frameSettings.frameFillColorPropertyItem
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
            navigationPanel: frameSettings.navigationPanel
            navigationRowStart: highlightColorSection.navigationRowEnd + 1

            visible: frameSettings.frameThicknessPropertyItem.isEnabled
            height: visible ? implicitHeight : 0

            titleText: qsTrc("inspector", "Thickness")
            propertyItem: frameSettings.frameThicknessPropertyItem

            step: 0.1
            minValue: 0
            maxValue: 5
        }

        SpinBoxPropertyView {
            id: marginSection
            anchors.left: parent.horizontalCenter
            anchors.leftMargin: 2
            anchors.right: parent.right

            navigationName: "Margin"
            navigationPanel: frameSettings.navigationPanel
            navigationRowStart: thicknessSection.navigationRowEnd + 1

            visible: frameSettings.frameMarginPropertyItem.isEnabled
            height: visible ? implicitHeight : 0

            titleText: qsTrc("inspector", "Margin")
            propertyItem: frameSettings.frameMarginPropertyItem

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
        navigationPanel: frameSettings.navigationPanel
        navigationRowStart: marginSection.navigationRowEnd + 1

        visible: frameSettings.frameCornerRadiusPropertyItem.isEnabled
        height: visible ? implicitHeight : 0

        titleText: qsTrc("inspector", "Corner radius")
        propertyItem: frameSettings.frameCornerRadiusPropertyItem

        step: 1
        decimals: 2
        minValue: 0
        maxValue: 100
    }
}
