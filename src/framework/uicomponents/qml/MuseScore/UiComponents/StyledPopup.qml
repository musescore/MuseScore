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

import MuseScore.Ui 1.0

Popup {
    id: root

    readonly property alias isOpened: prv.isOpened
    property bool opensUpward: false

    property Item anchorItem: null
    property real arrowX: Boolean(anchorItem) ? anchorItem.x - x + anchorItem.width / 2 : width / 2

    property alias navigation: navPanel
    property bool isDoActiveParentOnClose: true

    QtObject {
        id: prv

        property bool isOpened: false

        readonly property int padding: 12
        readonly property alias arrowHeight: arrow.height

        readonly property color backgroundColor: ui.theme.backgroundPrimaryColor
        readonly property color borderColor: ui.theme.strokeColor
        readonly property int borderWidth: 1

        onBackgroundColorChanged: { arrow.requestPaint() }
        onBorderColorChanged: { arrow.requestPaint() }
    }

    NavigationPopupPanel {
        id: navPanel
        enabled: root.visible && root.enabled
        order: {
            if (parentControl && parentControl.panel) {
                return parentControl.panel.order + 1
            }
            return -1
        }

        section: {
            if (parentControl && parentControl.panel) {
                return parentControl.panel.section
            }
            return null
        }

        parentControl: {
            if (root.anchorItem && root.anchorItem.navigation
                    && root.anchorItem.navigation instanceof NavigationControl) {
                return root.anchorItem.navigation
            }
            return null
        }

        onActiveChanged: {
            if (navPanel.active) {
                root.forceActiveFocus()
            } else {
                root.close()
            }
        }

        onNavigationEvent: function(event) {
            if (event.type === NavigationEvent.Escape) {
                root.close()
            }
        }
    }

    x: Boolean(anchorItem) ? Math.max(anchorItem.x + (anchorItem.width - width) / 2, 0) : 0
    y: Boolean(anchorItem) ? (opensUpward ? anchorItem.y - height : anchorItem.y + anchorItem.height) : 0

    topPadding: opensUpward ? prv.padding : prv.padding + prv.arrowHeight
    bottomPadding: opensUpward ? prv.padding + prv.arrowHeight : prv.padding
    leftPadding: prv.padding
    rightPadding: prv.padding

    onOpened: {
        prv.isOpened = true
    }
    onClosed: {
        prv.isOpened = false
        if (root.isDoActiveParentOnClose && navigation.parentControl) {
            navigation.parentControl.requestActive()
        }
    }

    closePolicy: Popup.CloseOnPressOutsideParent | Popup.CloseOnPressOutside | Popup.CloseOnEscape

    enter: Transition {
        NumberAnimation { property: "scale"; from: 0.9; to: 1.0; easing.type: Easing.OutQuint; duration: 220 }
        NumberAnimation { property: "opacity"; from: 0.0; to: 1.0; easing.type: Easing.OutCubic; duration: 150 }
    }

    exit: Transition {
        NumberAnimation { property: "scale"; from: 1.0; to: 0.9; easing.type: Easing.OutQuint; duration: 220 }
        NumberAnimation { property: "opacity"; from: 1.0; to: 0.0; easing.type: Easing.OutCubic; duration: 150 }
    }

    background: Item {
        anchors.fill: parent

        Item {
            id: mainBackground
            anchors.fill: parent

            Canvas {
                id: arrow
                anchors.top: root.opensUpward ? undefined : parent.top
                anchors.bottom: root.opensUpward ? parent.bottom : undefined
                z: 1
                height: 12
                width: 22
                x: Math.floor(root.arrowX - width / 2)

                onPaint: {
                    var ctx = getContext("2d");
                    ctx.fillStyle = prv.backgroundColor;
                    ctx.strokeStyle = prv.borderColor;
                    ctx.lineWidth = prv.borderWidth;
                    ctx.beginPath();

                    if (root.opensUpward) {
                        ctx.moveTo(0, 0);
                        ctx.lineTo(width / 2, height - 1);
                        ctx.lineTo(width, 0);
                    } else {
                        ctx.moveTo(0, height);
                        ctx.lineTo(width / 2, 1);
                        ctx.lineTo(width, height);
                    }

                    ctx.stroke();
                    ctx.fill();
                }
            }

            Rectangle {
                color: prv.backgroundColor
                border.color: prv.borderColor
                border.width: prv.borderWidth
                radius: 3

                anchors.top: root.opensUpward ? parent.top : arrow.bottom
                anchors.topMargin: root.opensUpward ? 0 : -1
                anchors.bottom: root.opensUpward ? arrow.top : parent.bottom
                anchors.bottomMargin: root.opensUpward ? -1 : 0
                anchors.left: parent.left
                anchors.right: parent.right
            }
        }

        StyledDropShadow {
            anchors.fill: parent
            source: mainBackground
        }
    }
}
