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
import QtQuick.Window 2.15

import MuseScore.UiComponents 1.0

import "../../"

AppWindow {
    id: root

    flags: Qt.FramelessWindowHint | Qt.Window
    color: "transparent"

    property int sideMargin: root.visibility === Window.Windowed ? 8 : 0

    function toggleMaximized() {
        if (root.visibility === Window.Maximized) {
            root.showNormal()
        } else {
            root.showMaximized()
        }
    }

    MouseArea {
        anchors.fill: parent

        acceptedButtons: Qt.NoButton
        hoverEnabled: true

        cursorShape: {
            const mouse = Qt.point(mouseX, mouseY)
            const borderOffset = root.sideMargin + 10
            if (mouse.x < borderOffset && mouse.y < borderOffset) return Qt.SizeFDiagCursor
            if (mouse.x >= width - borderOffset && mouse.y >= height - borderOffset) return Qt.SizeFDiagCursor
            if (mouse.x >= width - borderOffset && mouse.y < borderOffset) return Qt.SizeBDiagCursor
            if (mouse.x < borderOffset && mouse.y >= height - borderOffset) return Qt.SizeBDiagCursor
            if (mouse.x < borderOffset || mouse.x >= width - borderOffset) return Qt.SizeHorCursor
            if (mouse.y < borderOffset || mouse.y >= height - borderOffset) return Qt.SizeVerCursor

            return Qt.ArrowCursor
        }

        MouseArea {
            anchors.fill: parent
            anchors.margins: root.sideMargin
            enabled: false
        }
    }

    DragHandler {
        id: resizeHandler

        grabPermissions: TapHandler.TakeOverForbidden
        target: null

        onActiveChanged: {
            if (!active) {
                return
            }

            const p = resizeHandler.centroid.position
            var e = 0
            if (p.x / root.width < 0.10) { e |= Qt.LeftEdge }
            if (p.x / root.width > 0.90) { e |= Qt.RightEdge }
            if (p.y / root.height < 0.10) { e |= Qt.TopEdge }
            if (p.y / root.height > 0.90) { e |= Qt.BottomEdge }
            root.startSystemResize(e)
        }
    }

    Item {
        id: content

        anchors.fill: parent

        anchors.margins: root.sideMargin

        AppTitleBarLinux {
            id: appTitleBar

            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right

            height: 48
            title: root.title

            onShowWindowMinimizedRequested: {
                root.showMinimized()
            }

            onToggleWindowMaximizedRequested: {
                root.toggleMaximized()
            }

            onCloseWindowRequested: {
                root.close()
            }

            onStartSystemMoveRequested: {
                root.startSystemMove()
            }
        }

        WindowContent {
            id: window

            anchors.top: appTitleBar.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
        }
    }

    StyledDropShadow {
        anchors.fill: content
        source: content
        samples: 20
    }
}
