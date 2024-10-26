/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sergio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

import QtQuick 2.9
import com.kdab.dockwidgets 2.0

Item {
    id: root
    anchors.fill: parent
    readonly property int outterMargin: 10
    readonly property int innerMargin: 10
    readonly property QtObject innerIndicators: innerIndicators
    visible: width > 50 && height > 50 // don't show if window is too small'

    ClassicIndicator {
        visible: _kddw_overlayWindow && _kddw_overlayWindow.outterLeftIndicatorVisible
        indicatorType: KDDockWidgets.DropLocation_OutterLeft
        anchors {
            left: parent.left
            leftMargin: outterMargin
            verticalCenter: parent.verticalCenter
        }
    }

    ClassicIndicator {
        visible: _kddw_overlayWindow && _kddw_overlayWindow.outterRightIndicatorVisible
        indicatorType: KDDockWidgets.DropLocation_OutterRight
        anchors {
            right: parent.right
            rightMargin: outterMargin
            verticalCenter: parent.verticalCenter
        }
    }

    ClassicIndicator {
        visible: _kddw_overlayWindow && _kddw_overlayWindow.outterTopIndicatorVisible
        indicatorType: KDDockWidgets.DropLocation_OutterTop
        anchors {
            top: parent.top
            topMargin: outterMargin
            horizontalCenter: parent.horizontalCenter
        }
    }

    ClassicIndicator {
        visible: _kddw_overlayWindow && _kddw_overlayWindow.outterBottomIndicatorVisible
        indicatorType: KDDockWidgets.DropLocation_OutterBottom
        anchors {
            bottom: parent.bottom
            bottomMargin: outterMargin
            horizontalCenter: parent.horizontalCenter
        }
    }

    Item {
        id: innerIndicators
        objectName: "innerIndicators"

        x: _kddw_overlayWindow ? (_kddw_overlayWindow.hoveredGroupRect.x + (_kddw_overlayWindow.hoveredGroupRect.width / 2)) : 0
        y: _kddw_overlayWindow ? (_kddw_overlayWindow.hoveredGroupRect.y + (_kddw_overlayWindow.hoveredGroupRect.height / 2)) : 0

        width: (centerIndicator * 3) + (2 * innerMargin)
        height: width
        visible: _kddw_overlayWindow && (_kddw_overlayWindow.innerLeftIndicatorVisible || _kddw_overlayWindow.innerRightIndicatorVisible ||
                 _kddw_overlayWindow.innerTopIndicatorVisible || _kddw_overlayWindow.innerBottomIndicatorVisible || _kddw_overlayWindow.tabIndicatorVisible)

        ClassicIndicator {
            id: innerLeft
            visible: _kddw_overlayWindow && _kddw_overlayWindow.innerLeftIndicatorVisible
            indicatorType: KDDockWidgets.DropLocation_Left
            anchors {
                right: centerIndicator.left
                rightMargin: innerMargin
                verticalCenter: parent.verticalCenter
            }
        }

        ClassicIndicator {
            id: centerIndicator
            visible: _kddw_overlayWindow && _kddw_overlayWindow.tabIndicatorVisible
            indicatorType: KDDockWidgets.DropLocation_Center
            anchors.centerIn: parent
        }

        ClassicIndicator {
            id: innerRight
            visible: _kddw_overlayWindow && _kddw_overlayWindow.innerRightIndicatorVisible
            indicatorType: KDDockWidgets.DropLocation_Right
            anchors {
                left: centerIndicator.right
                leftMargin: innerMargin
                verticalCenter: parent.verticalCenter
            }
        }

        ClassicIndicator {
            id: innerTop
            visible: _kddw_overlayWindow && _kddw_overlayWindow.innerTopIndicatorVisible
            indicatorType: KDDockWidgets.DropLocation_Top
            anchors {
                bottom: centerIndicator.top
                bottomMargin: innerMargin
                horizontalCenter: parent.horizontalCenter
            }
        }

        ClassicIndicator {
            id: innerBottom
            visible: _kddw_overlayWindow && _kddw_overlayWindow.innerBottomIndicatorVisible
            indicatorType: KDDockWidgets.DropLocation_Bottom
            anchors {
                top: centerIndicator.bottom
                topMargin: innerMargin
                horizontalCenter: parent.horizontalCenter
            }
        }
    }
}
