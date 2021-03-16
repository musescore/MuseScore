//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2021 MuseScore BVBA and others
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

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtGraphicalEffects 1.0

import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0

PopupView {
    id: root

    property bool opensUpward: false
    property var arrowX: width / 2
    property alias arrowHeight: arrow.height
    property alias arrowVisible: arrow.visible

    property color borderColor: ui.theme.strokeColor
    property color fillColor: ui.theme.backgroundPrimaryColor
    readonly property int borderWidth: 1

    property bool animationEnabled: true

    closePolicy: PopupView.CloseOnPressOutsideParent

    onAboutToShow: {
        //!Note For some reason the call of mapToGlobal in the QML scope and in C++ on the same object produces different results
        //      The only reliable option is QML version, that's why we have to do get globalPos from QML for now.
        //      Makes sense to check it again on next QT update
        globalPos = mapToGlobal(parent.x + positionDisplacementX, parent.y + positionDisplacementY)
    }

    positionDisplacementX: parent.width / 2 - width / 2
    positionDisplacementY: opensUpward ? -height : parent.height

    padding: 24
    property int margins: 12

    backgroundItem: Item {
        anchors.fill: parent
        anchors.topMargin: opensUpward ? margins : 0
        anchors.leftMargin: margins
        anchors.rightMargin: margins
        anchors.bottomMargin: opensUpward ? 0 : margins

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
                x: Math.floor(root.arrowX - 12 - (width / 2))

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
                    top: opensUpward ? parent.top : (arrow.visible ? arrow.bottom : parent.top)
                    bottom: opensUpward ? (arrow.visible ? arrow.top : parent.bottom) : parent.bottom
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

    states: [
        State {
            name: "OPENED"
            when: root.isOpened

            PropertyChanges {
                target: root.backgroundItem
                scale: 1.0
                opacity: 1.0
            }

            PropertyChanges {
                target: root.contentItem
                scale: 1.0
                opacity: 1.0
            }
        },

        State {
            name: "CLOSED"
            when: !root.isOpened

            PropertyChanges {
                target: root.backgroundItem
                scale: 0.0
                opacity: 0.0
            }

            PropertyChanges {
                target: root.contentItem
                scale: 0.0
                opacity: 0.0
            }
        }
    ]

    transitions: Transition {
        NumberAnimation { properties: "scale, opacity"; easing.type: Easing.OutQuint; duration: animationEnabled ? 140 : 0 }
    }
}
