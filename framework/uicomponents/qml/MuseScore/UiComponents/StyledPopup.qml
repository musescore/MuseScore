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

    property var arrowX: width / 2
    property alias arrowHeight: arrow.height

    topPadding: bottomPadding + arrow.height

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
                    ctx.moveTo(0, height);
                    ctx.lineTo(width / 2, 1);
                    ctx.lineTo(width, height);
                    ctx.stroke();
                    ctx.fill();
                }
            }

            Rectangle {
                color: fillColor
                border { width: 1; color: borderColor }
                anchors {
                    top: arrow.bottom
                    topMargin: -1
                    bottom: parent.bottom
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
