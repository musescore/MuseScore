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

    property color borderColor: ui.theme.strokeColor
    property color fillColor: ui.theme.backgroundPrimaryColor
    readonly property int borderWidth: 1

    closePolicy: PopupView.CloseOnPressOutsideParent

    onAboutToShow: {
        globalPos = mapToGlobal(x, y)
    }

    backgroundItem: Item {
        anchors.fill: parent
        anchors.leftMargin: 12
        anchors.rightMargin: 12
        anchors.bottomMargin: 12

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
        NumberAnimation { properties: "scale, opacity"; easing.type: Easing.OutQuint; duration: 140 }
    }
}
