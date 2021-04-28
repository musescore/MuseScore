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

import com.kdab.dockwidgets 1.0

Item {
    id: root

    readonly property int outterMargin: 10
    readonly property int innerMargin: 10
    readonly property QtObject innerIndicators: innerIndicators

    anchors.fill: parent

    DropIndicator {
        anchors {
            left: parent.left
            leftMargin: root.outterMargin
            verticalCenter: parent.verticalCenter
        }

        indicatorType: DropIndicatorOverlayInterface.DropLocation_OutterLeft
        visible: dockDropIndicatorsWindow.dropIndicators.outterLeftIndicatorVisible
    }

    DropIndicator {
        anchors {
            right: parent.right
            rightMargin: root.outterMargin
            verticalCenter: parent.verticalCenter
        }

        indicatorType: DropIndicatorOverlayInterface.DropLocation_OutterRight
        visible: dockDropIndicatorsWindow.dropIndicators.outterRightIndicatorVisible
    }

    DropIndicator {
        anchors {
            top: parent.top
            topMargin: root.outterMargin
            horizontalCenter: parent.horizontalCenter
        }

        indicatorType: DropIndicatorOverlayInterface.DropLocation_OutterTop
        visible: dockDropIndicatorsWindow.dropIndicators.outterTopIndicatorVisible
    }

    DropIndicator {
        anchors {
            bottom: parent.bottom
            bottomMargin: root.outterMargin
            horizontalCenter: parent.horizontalCenter
        }

        indicatorType: DropIndicatorOverlayInterface.DropLocation_OutterBottom
        visible: dockDropIndicatorsWindow.dropIndicators.outterBottomIndicatorVisible
    }

    Item {
        id: innerIndicators

        objectName: "innerIndicators"

        x: dockDropIndicatorsWindow.dropIndicators.hoveredFrameRect.x + (dockDropIndicatorsWindow.dropIndicators.hoveredFrameRect.width / 2)
        y: dockDropIndicatorsWindow.dropIndicators.hoveredFrameRect.y + (dockDropIndicatorsWindow.dropIndicators.hoveredFrameRect.height / 2)

        width: (centerIndicator * 3) + (2 * innerMargin)
        height: width

        DropIndicator {
            id: centerIndicator

            anchors.centerIn: parent

            indicatorType: DropIndicatorOverlayInterface.DropLocation_Center
            visible: dockDropIndicatorsWindow.dropIndicators.centralIndicatorVisible
        }

        DropIndicator {
            anchors {
                right: centerIndicator.left
                rightMargin: root.innerMargin
                verticalCenter: parent.verticalCenter
            }

            indicatorType: DropIndicatorOverlayInterface.DropLocation_Left
            visible: dockDropIndicatorsWindow.dropIndicators.innerLeftIndicatorVisible
        }

        DropIndicator {
            anchors {
                left: centerIndicator.right
                leftMargin: root.innerMargin
                verticalCenter: parent.verticalCenter
            }

            indicatorType: DropIndicatorOverlayInterface.DropLocation_Right
            visible: dockDropIndicatorsWindow.dropIndicators.innerRightIndicatorVisible
        }

        DropIndicator {
            anchors {
                bottom: centerIndicator.top
                bottomMargin: root.innerMargin
                horizontalCenter: parent.horizontalCenter
            }

            indicatorType: DropIndicatorOverlayInterface.DropLocation_Top
            visible: dockDropIndicatorsWindow.dropIndicators.innerTopIndicatorVisible
        }

        DropIndicator {
            anchors {
                top: centerIndicator.bottom
                topMargin: root.innerMargin
                horizontalCenter: parent.horizontalCenter
            }

            indicatorType: DropIndicatorOverlayInterface.DropLocation_Bottom
            visible: dockDropIndicatorsWindow.dropIndicators.innerBottomIndicatorVisible
        }
    }
}

