import QtQuick 2.15
import MuseScore.Ui 1.0

Item {
    id: root

    property var icon

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

                pressAndHoldInterval: 200

                onClicked: {
                    root.increaseButtonClicked()
                }

                onPressAndHold: {
                    continuousIncreaseTimer.running = true
                }

                onReleased: {
                    continuousIncreaseTimer.running = false
                }

                Timer {
                    id: continuousIncreaseTimer

                    interval: 100

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

                preventStealing: true

                pressAndHoldInterval: 200

                onClicked: {
                    root.decreaseButtonClicked()
                }

                onPressAndHold: {
                    continuousDecreaseTimer.running = true
                }

                onReleased: {
                    continuousDecreaseTimer.running = false
                }

                Timer {
                    id: continuousDecreaseTimer

                    interval: 100

                    repeat: true

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

            color: "transparent"

            width: buttonIcon.width
            height: buttonIcon.height

            StyledIconLabel {
                id: buttonIcon

                iconCode: root.icon
            }
        }
    }
}
