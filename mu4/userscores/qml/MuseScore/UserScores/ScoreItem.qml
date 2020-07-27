import QtQuick 2.7
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
            height: 224
            width: 172

            radius: 3

            Loader {
                id: loader

                anchors.fill: parent
                anchors.margins: parent.radius

                property var thumbnail: undefined

                sourceComponent: isAdd ? addComp : thumbnailComp

                onLoaded: {
                    if (!isAdd) {
                        item.setThumbnail(thumbnail)
                    }
                }
            }
        }

        Column {
            anchors.horizontalCenter: parent.horizontalCenter

            spacing: 4

            StyledTextLabel {
                anchors.horizontalCenter: parent.horizontalCenter

                text: root.title
            }

            StyledTextLabel {
                id: timeSinceCreation

                anchors.horizontalCenter: parent.horizontalCenter

                font.pixelSize: 12
                font.capitalization: Font.AllUppercase

                visible: !isAdd
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
        anchors.fill: parent

        onClicked: {
            root.clicked()
        }
    }
}
