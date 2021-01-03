//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

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

    property var arrowX: width / 2
    property alias arrowHeight: arrow.height

    readonly property int borderWidth: 1

    property Item anchorItem: null

    y: opensUpward ? anchorItem.y - height : anchorItem.y + anchorItem.height

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

        DropShadow {
            anchors.fill: parent
            source: mainBackground
            color: "#75000000"
            verticalOffset: 4
            samples: 30
        }
    }
}
