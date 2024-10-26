/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

import QtQuick 2.9
import QtQuick.Layouts 1.9

import com.kdab.dockwidgets 2.0

Rectangle {
    id: root

    property QtObject groupCpp
    readonly property QtObject titleBarCpp: groupCpp ? groupCpp.titleBar : null
    readonly property int nonContentsHeight: (titleBar.item ? titleBar.item.heightWhenVisible : 0) + tabbar.implicitHeight + (2 * contentsMargin) + titleBarContentsMargin
    property int contentsMargin: isMDI ? 2 : 1
    property int titleBarContentsMargin: 1
    property int mouseResizeMargin: 8
    readonly property bool isMDI: groupCpp && groupCpp.isMDI
    readonly property bool resizeAllowed: root.isMDI && !_kddwHelpers.isDragging && _kddwDockRegistry && (!_kddwHelpers.groupViewInMDIResize || _kddwHelpers.groupViewInMDIResize === groupCpp)
    property alias tabBarHeight: tabbar.height
    readonly property bool hasCustomMouseEventRedirector: false
    readonly property bool isFixedHeight: groupCpp && groupCpp.isFixedHeight
    readonly property bool isFixedWidth: groupCpp && groupCpp.isFixedWidth

    anchors.fill: parent

    radius: 2
    color: "transparent"
    border {
        color: "#b8b8b8"
        width: 1
    }

    onGroupCppChanged: {
        if (groupCpp) {
            groupCpp.setStackLayout(stackLayout);
        }
    }

    onNonContentsHeightChanged: {
        if (groupCpp)
            groupCpp.geometryUpdated();
    }

    MouseArea {
        anchors.fill: parent


        MDIResizeHandlerHelper {
            anchors {
                left: parent ? parent.left : undefined
                top: parent ? parent.top : undefined
                bottom: parent ? parent.bottom : undefined
            }

            width: resizeMargin
            z: 100
            groupCpp: root.groupCpp
            resizeAllowed: root.resizeAllowed && !root.isFixedWidth
            resizeMargin: root.mouseResizeMargin
            cursorPosition: KDDockWidgets.CursorPosition_Left
        }

        MDIResizeHandlerHelper {
            anchors {
                right: parent ? parent.right : undefined
                top: parent ? parent.top : undefined
                bottom: parent ? parent.bottom : undefined
            }

            width: resizeMargin
            z: 100
            groupCpp: root.groupCpp
            resizeAllowed: root.resizeAllowed && !root.isFixedWidth
            resizeMargin: root.mouseResizeMargin
            cursorPosition: KDDockWidgets.CursorPosition_Right
        }

        MDIResizeHandlerHelper {
            anchors {
                right: parent ? parent.right : undefined
                top: parent ? parent.top : undefined
                left: parent ? parent.left : undefined
            }

            height: resizeMargin
            z: 100
            groupCpp: root.groupCpp
            resizeAllowed: root.resizeAllowed && !root.isFixedHeight
            resizeMargin: root.mouseResizeMargin
            cursorPosition: KDDockWidgets.CursorPosition_Top
        }

        MDIResizeHandlerHelper {
            anchors {
                right: parent ? parent.right : undefined
                left: parent ? parent.left : undefined
                bottom: parent ?  parent.bottom : undefined
            }

            height: resizeMargin
            z: 100
            groupCpp: root.groupCpp
            resizeAllowed: root.resizeAllowed && !root.isFixedHeight
            resizeMargin: root.mouseResizeMargin
            cursorPosition: KDDockWidgets.CursorPosition_Bottom
        }

        MDIResizeHandlerHelper {
            anchors {
                right: parent ? parent.right : undefined
                bottom: parent ? parent.bottom : undefined
            }

            height: width
            width: resizeMargin
            z: 101
            groupCpp: root.groupCpp
            resizeAllowed: root.resizeAllowed
            resizeMargin: root.mouseResizeMargin
            cursorPosition: KDDockWidgets.CursorPosition_Bottom | KDDockWidgets.CursorPosition_Right
        }

        MDIResizeHandlerHelper {
            anchors {
                left:  parent ? parent.left : undefined
                top:  parent ? parent.top : undefined
            }

            height: width
            width: resizeMargin
            z: 101
            groupCpp: root.groupCpp
            resizeAllowed: root.resizeAllowed
            resizeMargin: root.mouseResizeMargin
            cursorPosition: KDDockWidgets.CursorPosition_Top | KDDockWidgets.CursorPosition_Left
        }

        MDIResizeHandlerHelper {
            anchors {
                right: parent ? parent.right : undefined
                top: parent ? parent.top : undefined
            }

            height: width
            width: resizeMargin
            z: 101
            groupCpp: root.groupCpp
            resizeAllowed: root.resizeAllowed
            resizeMargin: root.mouseResizeMargin
            cursorPosition: KDDockWidgets.CursorPosition_Top | KDDockWidgets.CursorPosition_Right
        }

        MDIResizeHandlerHelper {
            anchors {
                left: parent ? parent.left : undefined
                bottom: parent ? parent.bottom : undefined
            }

            height: width
            width: resizeMargin
            z: 101
            groupCpp: root.groupCpp
            resizeAllowed: root.resizeAllowed
            resizeMargin: root.mouseResizeMargin
            cursorPosition: KDDockWidgets.CursorPosition_Left | KDDockWidgets.CursorPosition_Bottom
        }
    }

    Loader {
        id: titleBar
        readonly property QtObject titleBarCpp: root.titleBarCpp
        source: groupCpp ? _kddw_widgetFactory.titleBarFilename()
                         : ""

        anchors {
            top:  parent ? parent.top : undefined
            left: parent ? parent.left : undefined
            right: parent ? parent.right : undefined
            topMargin: root.titleBarContentsMargin
            leftMargin: root.titleBarContentsMargin
            rightMargin: root.titleBarContentsMargin
        }
    }

    Loader {
        id: tabbar
        readonly property QtObject groupCpp: root.groupCpp
        readonly property bool hasCustomMouseEventRedirector: root.hasCustomMouseEventRedirector

        source: groupCpp ? _kddw_widgetFactory.tabbarFilename()
                         : ""

        anchors {
            left: parent ? parent.left : undefined
            right: parent ? parent.right : undefined
            top: (titleBar && titleBar.visible) ? titleBar.bottom
                                                : (parent ? parent.top : undefined)
            topMargin: 1
            leftMargin: 1
            rightMargin: 1
        }
    }

    Item {
        id: stackLayout
        anchors {
            left: parent ? parent.left : undefined
            right: parent ? parent.right : undefined
            top: (parent && tabbar.visible) ? tabbar.bottom : ((titleBar && titleBar.visible) ? titleBar.bottom
                                                                                              : parent ? parent.top : undefined)
            bottom: parent ? parent.bottom : undefined

            leftMargin: root.contentsMargin
            rightMargin: root.contentsMargin
            bottomMargin: root.contentsMargin
        }
    }
}
