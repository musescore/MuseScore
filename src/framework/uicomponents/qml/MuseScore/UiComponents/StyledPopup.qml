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

import QtQuick 2.8
import QtQuick.Controls 2.1
import QtGraphicalEffects 1.0
import MuseScore.Ui 1.0

Popup {
    id: root

    property color borderColor: ui.theme.strokeColor
    property color fillColor: ui.theme.backgroundPrimaryColor
    property bool isOpened: false
    property bool opensUpward: false

    property var arrowX: Boolean(anchorItem) ? anchorItem.x - x + anchorItem.width / 2 : width / 2
    property alias arrowHeight: arrow.height

    readonly property int borderWidth: 1

    property Item anchorItem: null

    x: Boolean(anchorItem) ? Math.max(anchorItem.x + (anchorItem.width - width) / 2, 0) : 0
    y: Boolean(anchorItem) ? (opensUpward ? anchorItem.y - height : anchorItem.y + anchorItem.height) : 0

    property var popupPadding: 12
    topPadding: opensUpward ? popupPadding : popupPadding + arrowHeight
    bottomPadding: opensUpward ? popupPadding + arrowHeight : popupPadding
    leftPadding: popupPadding
    rightPadding: popupPadding

    onOpened: { isOpened = true }
    onClosed: { isOpened = false }

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
                anchors.top: opensUpward ? undefined : parent.top
                anchors.bottom: opensUpward ? parent.bottom : undefined
                z: 1
                height: 12
                width: 22
                x: Math.floor(root.arrowX - width / 2)

                onPaint: {
                    var ctx = getContext("2d");
                    ctx.lineWidth = 2;
                    ctx.fillStyle = fillColor;
                    ctx.strokeStyle = borderColor;
                    ctx.beginPath();

                    if (opensUpward) {
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
                color: fillColor
                border { width: root.borderWidth; color: root.borderColor }
                radius: 3

                anchors {
                    top: opensUpward ? parent.top : arrow.bottom
                    topMargin: opensUpward ? 0 : -1
                    bottom: opensUpward ? arrow.top : parent.bottom
                    bottomMargin: opensUpward ? -1 : 0
                    left: parent.left
                    right: parent.right
                }
            }
        }

        StyledDropShadow {
            anchors.fill: parent
            source: mainBackground
        }
    }
}
