import QtQuick 2.0
import MuseScore.Ui 1.0

Item {
    id: root

    property var icon
    property int iconPixelSize: 16

    signal increaseButtonClicked
    signal decreaseButtonClicked

    height: childrenRect.height
    width: childrenRect.width

    Column {
        id: adjustButtonsColumn

        height: childrenRect.height
        width: childrenRect.width

        Loader {
            id: incrementButtonLoader

            height: childrenRect.height
            width: childrenRect.width

            rotation: 180

            sourceComponent: adjustButtonComponent

            MouseArea {
                id: increaseMouseArea

                anchors.fill: parent

                preventStealing: true

                onClicked: {
                    root.increaseButtonClicked()
                }

                onPressAndHold: {
                    continiousIncreaseTimer.running = true
                }

                onReleased: {
                    continiousIncreaseTimer.running = false
                }

                Timer {
                    id: continiousIncreaseTimer

                    interval: 300

                    repeat: true

                    onTriggered: {
                        root.increaseButtonClicked()
                    }
                }
            }
        }

        Loader {
            id: decrementButtonLoader

            height: childrenRect.height
            width: childrenRect.width

            sourceComponent: adjustButtonComponent

            MouseArea {
                id: decreaseMouseArea

                anchors.fill: parent

                onClicked: {
                    root.decreaseButtonClicked()
                }

                onPressAndHold: {
                    continiousDecreaseTimer.running = true
                }

                onReleased: {
                    continiousDecreaseTimer.running = false
                }

                Timer {
                    id: continiousDecreaseTimer

                    interval: 300
                    repeat: true

                    running: decreaseMouseArea.pressed

                    onTriggered: {
                        root.decreaseButtonClicked()
                    }
                }
            }
        }
    }

    Component {
        id: adjustButtonComponent

        Rectangle {
            id: backgroundRect

            height: iconPixelSize
            width: iconPixelSize

            color: "transparent"

            StyledIconLabel {
                id: buttonIcon

                anchors.fill: parent
                iconCode: root.icon
            }
        }
    }
}
