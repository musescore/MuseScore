import QtQuick 2.7
import QtGraphicalEffects 1.0

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.UserScores 1.0

Item {
    id: root

    property string title: ""
    property alias timeSinceCreation: timeSinceCreation.text
    property alias thumbnail: loader.thumbnail
    property bool isAdd: false

    signal clicked()

    Column {
        anchors.fill: parent

        spacing: 16

        Rectangle {
            id: scoreRect

            height: 224
            width: 172

            radius: 3

            opacity: 0.9

            border.width: 0
            border.color: ui.theme.strokeColor

            Loader {
                id: loader

                anchors.fill: parent
                anchors.margins: parent.radius

                property var thumbnail: undefined

                sourceComponent: root.isAdd ? addComp : thumbnailComp

                onLoaded: {
                    if (!root.isAdd) {
                        item.setThumbnail(root.thumbnail)
                    }
                }
            }

            states: [
                State {
                    name: "HOVERED"
                    when: mouseArea.containsMouse && !mouseArea.pressed

                    PropertyChanges {
                        target: scoreRect
                        opacity: 1
                        border.width: 1
                    }
                },

                State {
                    name: "PRESSED"
                    when: mouseArea.pressed

                    PropertyChanges {
                        target: scoreRect
                        opacity: 0.5
                        color: "transparent"
                    }
                }
            ]

            RectangularGlow {
                anchors.fill: scoreRect
                z: -1

                glowRadius: 20
                color: "#08000000"
                cornerRadius: scoreRect.radius + glowRadius
            }
        }

        Column {
            anchors.horizontalCenter: parent.horizontalCenter

            spacing: 4

            StyledTextLabel {
                anchors.horizontalCenter: parent.horizontalCenter

                text: root.title

                font.pixelSize: 14
            }

            StyledTextLabel {
                id: timeSinceCreation

                anchors.horizontalCenter: parent.horizontalCenter

                font.pixelSize: 12
                font.capitalization: Font.AllUppercase

                visible: !root.isAdd
            }
        }
    }

    Component {
        id: thumbnailComp

        ScoreThumbnail {
            anchors.fill: parent
        }
    }

    Component {
        id: addComp

        Rectangle {
            anchors.fill: parent

            color: "#FFFFFF"

            StyledIconLabel {
                anchors.centerIn: parent

                iconCode: IconCode.PLUS

                font.pixelSize: 50
            }
        }
    }

    MouseArea {
        id: mouseArea

        anchors.fill: parent

        hoverEnabled: true

        onClicked: {
            root.clicked()
        }
    }
}
