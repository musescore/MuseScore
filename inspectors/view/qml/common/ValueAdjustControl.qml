import QtQuick 2.0

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

                onClicked: {
                    root.increaseButtonClicked()
                }

                hoverEnabled: true

                onHoveredChanged: {
                    if (containsMouse)
                        incrementButtonLoader.item.color = globalStyle.highlight
                    else
                        incrementButtonLoader.item.color = "transparent"
                }

                onReleased: {
                    continuousIncreaseTimer.running = false
                    continuousIncreaseTimer.interval = 300
                }

                onPressAndHold: {
                    continuousIncreaseTimer.running = true
                }

                Timer {
                    id: continuousIncreaseTimer

                    interval: 300
                    repeat: true

                    onTriggered: {
                        root.increaseButtonClicked()
                        interval = Math.max(interval - 10, 50) // accelerate
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

                hoverEnabled: true
                onHoveredChanged: {
                    if (containsMouse)
                        decrementButtonLoader.item.color = globalStyle.highlight
                    else
                        decrementButtonLoader.item.color = "transparent"
                }

                onReleased: {
                    continuousDecreaseTimer.running = false;
                    continuousDecreaseTimer.interval = 300
                }

                onPressAndHold: {
                    continuousDecreaseTimer.running = true
                }

                Timer {
                    id: continuousDecreaseTimer

                    interval: 300
                    repeat: true

                    onTriggered: {
                        root.decreaseButtonClicked()
                        interval = Math.max(interval - 10, 50) // accelerate
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
            radius: 1

            Behavior on color { ColorAnimation { duration: 100 } }

            StyledIconLabel {
                id: buttonIcon

                anchors.fill: parent
                iconCode: root.icon
            }
        }
    }
}
